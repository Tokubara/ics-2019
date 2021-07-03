#include "fs.h"

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset; // 添加的读写offset
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

// {{{1 device声明和file_table
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin", 0, 0, invalid_read, invalid_write},
  {"stdout", 0, 0, invalid_read, serial_write},
  {"stderr", 0, 0, invalid_read, serial_write},
  {"/dev/events", 0, 0, events_read, invalid_write},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

int fs_open(const char *pathname, int flags, int mode) {
  int fd = -1;
  for(size_t i = 0; i < NR_FILES; i++) {
    if(strcmp(file_table[i].name, pathname) == 0) {
      fd = i;
      file_table[i].open_offset = 0; // 其实这是多余的, 如果真的不为0, 怎么都会导致错误
      break;
    }
  }
  return fd;
}
#define min(a,b) (a<b?a:b)
#define check_fd   if(fd < 0 || fd >= NR_FILES) {\
    Log("error: invalid fd: %d", fd);\
    return -1;\
  }

ssize_t fs_read(int fd, void *buf, size_t len) {
  check_fd;
  if(fd < 0 || fd >= NR_FILES) {
    Log("error: invalid fd: %d", fd);
    return 0;
  }
  size_t read_len = min(len, file_table[fd].size - file_table[fd].open_offset);
  size_t disk_offset = file_table[fd].disk_offset + file_table[fd].open_offset;
  ramdisk_read(buf, disk_offset, read_len);
  file_table[fd].open_offset += read_len;
  return read_len;
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
  check_fd;
  ssize_t write_len;
  if(file_table[fd].write != NULL) {
    write_len = file_table[fd].write(buf, 0, len); // offset参数用不上, 写为0
  } else {
    write_len = min(len, file_table[fd].size - file_table[fd].open_offset);
    size_t disk_offset = file_table[fd].disk_offset + file_table[fd].open_offset;
    ramdisk_write(buf, disk_offset, write_len);
    file_table[fd].open_offset += write_len;
  }
  return write_len;
}

ssize_t fs_lseek(int fd, ssize_t offset, int whence) { // 讲义与man 2 lseek不一致, 是size_t, 应该是符号数类型
  check_fd;
  ssize_t new_offset = 0;
  switch(whence) {
    case SEEK_SET: {
                     new_offset = offset;
                     break;
                   }
    case SEEK_CUR: {
                     new_offset = file_table[fd].open_offset + offset;
                     break;
                   }
    case SEEK_END: {
                     new_offset = file_table[fd].size + offset;
                     break;
                   }
    default: {
               Log("unknown whence: %d", whence);
               new_offset = -1; // 表示不合法
                     break;
                   }
  }
  if(new_offset < 0 && new_offset > file_table[fd].size) {
    Log("invalid offset: %d, offset unchange", (int)new_offset);
    return file_table[fd].open_offset;
  }
  file_table[fd].open_offset = new_offset;
  return new_offset;
}

int fs_close(int fd) {
  return 0;
}

void init_fs() {
  // TODO: initialize the size of /dev/fb
}
