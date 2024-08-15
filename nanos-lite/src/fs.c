#include "klib-macros.h"
#include <fs.h>

//#define CONFIG_STRACE 1
#define ARRLEN(arr) (sizeof(arr) / sizeof(arr[0]))

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

enum {FD_STDIN, FD_STDOUT, FD_STDERR, DEV_EVENTS, DEV_DISPLAY, FD_FB, FD_SB, FD_SBCTL};

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);

extern size_t events_read(void *buf, size_t offset, size_t len);
extern size_t dispinfo_read(void *buf, size_t offset, size_t len);
extern size_t fb_write(const void *buf, size_t offset, size_t len);
extern size_t sb_write(const void *buf, size_t offset, size_t len);
extern size_t sbctl_read(void *buf, size_t offset, size_t len);
extern size_t sbctl_write(const void *buf, size_t offset, size_t len);


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
  [FD_SB] = {"/dev/sb", 0, 0, NULL, sb_write},
  [FD_SBCTL] = {"/dev/sbctl", 0, 0, sbctl_read, sbctl_write},
#include "files.h"
};

// it is not need to use in other files
// it should be zero, but some thing make it not zero
// am_native is right. so it is am or nanos problem
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

  printf("open_index is locate %p\n", &open_index);
  printf("open_index is %d\n", open_index);
  assert(open_index == 0);
  // always open the stream file.
  open_index = FD_SBCTL+1;
  for(int i = 0; i <= FD_SBCTL; i++)
  {
    open_table[i] = (OFinfo){.fd = i, .open_offset = 0};
  }

  file_table[FD_SB].size = io_read(AM_AUDIO_CONFIG).bufsize;
  file_table[FD_SBCTL].size = 3 * sizeof(int);
}

static int get_index(size_t fd)
{
  if (fd < 0 || fd > LENGTH(open_table))
  {
    return -1;
  }
//  if(fd <= FD_SBCTL)
//  {
//#ifdef CONFIG_STRACE
//    Log("ignore open %s", file_table[fd].name);
//#endif
//    return fd;
//  }
  int ret = -1;
  for(size_t i = 0; i < open_index; i++)
  {
    //printf("open_table[%d].fd is %d\n", i, open_table[i].fd);
    if(fd == open_table[i].fd)
    {
      ret = i;
      break;
    }
  }
  //if(ret == -1)
  //{
  //  printf("%d file not found! open_index is %d, all file is:\n", fd, open_index);
  //}
  return ret;
}

int fs_open(const char *pathname, int flags, int mode) {
#ifdef CONFIG_STRACE
  Log("need to open %s", pathname);
#endif
  for(int i = 0; i < LENGTH(file_table); i++) {
    //printf("file_table[%d].name: %s\n", i, file_table[i].name);
    if(strcmp(pathname, file_table[i].name) == 0) {
      if(i <= FD_SBCTL)
      {
#ifdef CONFIG_STRACE
        Log("ignore open %s", pathname);
#endif
        return i;
      }
      // record 
      //printf("file fd is %d\n", i);
      //printf("open_index is %d\n", open_index);
      assert(open_index < LENGTH(open_table));
      open_table[open_index].fd = i;
      open_table[open_index].open_offset = 0;
      open_index++;
#ifdef CONFIG_STRACE
      Log("have read %s", pathname);
#endif

      return i;
    }
  }
  panic("file %s not found", pathname);
  while(1);
  return -1;
}

int fs_close(int fd) {
  if(fd <= FD_SBCTL)
  {
    return 0;
  }
  int index = get_index(fd);
  if(index != -1)
  {
    open_table[index].fd = -1;
  }
  return 0;
}

size_t fs_read(int fd, void *buf, size_t len) {
  size_t index = get_index(fd);
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
#ifdef CONFIG_STRACE
    printf("[fs_read] file size is %d is less than offest %d + len %d\n", file_table[fd].size, offset, len);
#endif
    read_len = file_table[fd].size - offset;
  }
  size_t ret;
  //Log("[fs_read] fd is %d, file %s, disk_offset %d, offset %d, len %d", fd, file_table[fd].name, file_table[fd].disk_offset, offset, read_len);
  if(file_table[fd].read)
  {
    //printf("the file %s have read %d\n", file_table[fd].name, read_len);
    ret = file_table[fd].read(buf, file_table[fd].disk_offset + offset, len);
  }
  else
  {
    ret = ramdisk_read(buf, file_table[fd].disk_offset + offset, read_len);
    open_table[index].open_offset += ret;
    // if the file read end, reset it.
    //if(read_len != len)
    //{
    //  open_table[index].open_offset = 0;
    //}
  }
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
#ifdef CONFIG_STRACE
    printf("[fs_write] file size is %d is less than offest %d + len %d\n", file_table[fd].size, offset, len);
#endif
    read_len = file_table[fd].size - offset;
  }
#ifdef CONFIG_STRACE
  Log("[fs_write] fd is %d, file %s, disk_offset %d, offset %d, len %d", fd, file_table[fd].name, file_table[fd].disk_offset, offset, read_len);
#endif
  size_t ret;
  if(file_table[fd].write)
  {
    // the disk_offset will be proceed in the write function
    ret = file_table[fd].write(buf, file_table[fd].disk_offset + offset, len);
  }
  else{
    ret = ramdisk_write(buf, file_table[fd].disk_offset + offset, read_len);
    open_table[index].open_offset += ret;
    //if(read_len != len)
    //{
    //  open_table[index].open_offset = 0;
    //}
  }

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
  //printf("the file %s offset is %d\n", file_table[fd].name, open_table[index].open_offset);
  
  return open_table[index].open_offset;
}
