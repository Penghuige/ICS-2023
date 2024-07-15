#include "klib-macros.h"
#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

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

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

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
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, invalid_write},
#include "files.h"
};

// it need to use in other files
static int open_index = 0;
static OFinfo open_table[LENGTH(file_table)];

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

static int get_index(int fd)
{
  int ret = -1;
  for(int i = 0; i < open_index; i++)
  {
    //printf("name: %d\n", open_table[i].fd);
    if(fd == open_table[i].fd)
    {
      ret = i;
      break;
    }
  }
  return ret;
}

int fs_open(const char *pathname, int flags, int mode) {
  //for(int i = 0; i < LENGTH(file_table); i++) {
  //  printf("file_table[%d].name = %s\n", i, file_table[i].name);
  //}
  Log("open %s", pathname);
  for(int i = 0; i < LENGTH(file_table); i++) {
    if(strcmp(pathname, file_table[i].name) == 0) {
      if(i <= 2)
      {
        Log("ignore open %s", pathname);
        return i;
      }
      // record 
      printf("file fd is %d\n", i);
      open_table[open_index].fd = i;
      open_table[open_index].open_offset = 0;
      open_index++;
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
  printf("index: %d\n", index);
  if(index == -1)
  {
    panic("file %d not found", fd);
  }
  size_t offset = open_table[index].open_offset;
  size_t ret = file_table[fd].read(buf, offset, len);
  open_table[index].open_offset += ret;
  return ret;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  int index = get_index(fd);
  if(index == -1)
  {
    panic("file %d not found", fd);
  }
  size_t offset = open_table[index].open_offset;
  size_t ret = file_table[fd].write(buf, offset, len);
  open_table[index].open_offset += ret;
  return ret;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
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
