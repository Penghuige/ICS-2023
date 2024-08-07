#include "klib-macros.h"
#include <fs.h>

//#define CONFIG_STRACE 0

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

extern size_t serial_write(const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

// record open-file table, it need to store file name and offset
typedef struct {
  size_t fd;
  // where it must to write
  size_t open_offset;
}OFinfo;

// File system
int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);

enum {FD_STDIN, FD_STDOUT, FD_STDERR, DEV_EVENTS, DEV_DISPLAY, FD_FB};

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);

extern size_t events_read(void *buf, size_t offset, size_t len);
extern size_t dispinfo_read(void *buf, size_t offset, size_t len);
extern size_t fb_write(const void *buf, size_t offset, size_t len);


size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [DEV_EVENTS] = {"/dev/events", 0, 0, events_read, NULL},
  [DEV_DISPLAY] = {"/proc/dispinfo", 0, 0, dispinfo_read, NULL},
  [FD_FB] = {"/dev/fb", 0, 0, NULL, fb_write},
#include "files.h"
};

// it is not need to use in other files
static size_t open_index;
static OFinfo open_table[5*LENGTH(file_table)];

void init_fs() {
  // TODO: initialize the size of /dev/fb
  // make the special file here
  // size is the screen size * pixel size
  AM_GPU_CONFIG_T ev = io_read(AM_GPU_CONFIG);
  int width = ev.width;
  int height = ev.height;
  // pixel size is uint32_t
  file_table[FD_FB].size = width * height * sizeof(uint32_t);

  open_index = FD_FB+1;
  for(int i = 0; i <= FD_FB; i++)
  {
    open_table[i] = (OFinfo){.fd = i, .open_offset = 0};
  }
}

static int get_index(size_t fd)
{
  if (fd < 0 || fd > LENGTH(open_table))
  {
    return -1;
  }
  if(fd <= FD_FB)
  {
#ifdef CONFIG_STRACE
    Log("ignore open %s", file_table[fd].name);
#endif
    return fd;
  }
  int ret = -1;
  for(int i = 0; i < open_index; i++)
  {
    if(fd == open_table[i].fd)
    {
      ret = i;
      break;
    }
  }
  if(ret == -1)
  {
    printf("%d file not found! open_index is %d, all file is:\n", fd, open_index);
  }
  return ret;
}

int fs_open(const char *pathname, int flags, int mode) {
  Log("need to open %s", pathname);
  for(int i = 0; i < LENGTH(file_table); i++) {
    printf("file_table[%d].name: %s\n", i, file_table[i].name);
    if(strcmp(pathname, file_table[i].name) == 0) {
      if(i <= FD_FB)
      {
        Log("ignore open %s", pathname);
        return i;
      }
      // record 
      //printf("file fd is %d\n", i);
      //printf("open_index is %d\n", open_index);
      assert(open_index < LENGTH(open_table));
      open_table[open_index].fd = i;
      open_table[open_index].open_offset = 0;
      open_index++;
      Log("have read %s", pathname);

      return i;
    }
  }
  panic("file %s not found", pathname);
  while(1);
  return -1;
}

int fs_close(int fd) {
  int index = get_index(fd);
  if(index != -1)
  {
    open_table[fd].fd = -1;
  }
  return 0;
}

size_t fs_read(int fd, void *buf, size_t len) {
  int index = get_index(fd);
#ifdef CONFIG_STRACE
  Log("READ index: %d, name: %s, offset: %d", index, file_table[fd].name, open_table[index].open_offset);
#endif
  if(index == -1)
  {
    panic("file %d not found", fd);
  }
  size_t offset = open_table[index].open_offset;
  // printf("offset: %d\n", offset);
  // 他的buf就是指向的Elf_Ehdr ehdr, 第二个参数是偏移量，记录在文件表中的disk_offset，第三个参数是长度
  size_t read_len = len;
  // when the file size is not enough
  if (file_table[fd].size < offset + len) {
    read_len = file_table[fd].size - offset;
  }

  if(file_table[index].read)
  {
    return file_table[index].read(buf, offset, len);
  }
  size_t ret = ramdisk_read(buf, file_table[fd].disk_offset + offset, read_len);
  open_table[index].open_offset += ret;
  return ret;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  int index = get_index(fd);
#ifdef CONFIG_STRACE
  Log("WRITE index: %d, name: %s, offset: %d, len: %d", index, file_table[fd].name, open_table[index].open_offset, len);
#endif
  if(index == -1)
  {
    panic("file %d not found", fd);
  }
  size_t offset = open_table[index].open_offset;
  size_t read_len = len;
  // when the file size is not enough
  if(file_table[fd].size < offset + len)
  {
    read_len = file_table[fd].size - offset;
  }
  if(file_table[index].write)
  {
    return file_table[index].write(buf, offset, len);
  }
  size_t ret = ramdisk_write(buf, file_table[fd].disk_offset + offset, read_len);

  open_table[index].open_offset += ret;

  return ret;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
#ifdef CONFIG_STRACE
  Log("LSEEK offset: %d, whence: %d", offset, whence);
#endif
  int index = get_index(fd);
  if(index == -1)
  {
    panic("file %d not found", fd);
  }
  switch(whence)
  {
    case SEEK_SET:
      open_table[index].open_offset = offset;
      break;
    case SEEK_CUR:
      open_table[index].open_offset += offset;
      break;
    case SEEK_END:
      open_table[index].open_offset = file_table[fd].size + offset;
      break;
    default:
      panic("whence error");
  }
  
  return open_table[index].open_offset;
}
