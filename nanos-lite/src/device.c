#include "common.h"
#include <amdev.h>

size_t serial_write(const void *buf, size_t offset, size_t len) {
  (void)offset;
  _yield();
  char* ch_buf = (char*)buf;
  for(size_t i = 0; i < len; i++) {
    _putc(ch_buf[i]);
  }
  return len;
}

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

// 怀疑offset参数是有意义的, 因为dispinfo_read和它类似, 但offset参数都有作用
size_t events_read(void *buf, size_t offset, size_t len) {
  (void)offset;
  _yield();
  // 读键盘
  // printf("events_read:%d\n", len);
  size_t write_len = 0;
  int key_code = read_key();
  char* ch_buf = (char*)buf;
  if(key_code != _KEY_NONE) {
    int key_down = ((key_code&0x8000)!=0);
    key_code &= ~0x8000;
    write_len = snprintf(ch_buf, len, "k%c %s\n", (key_down)?'d':'u', names[key_code]);
  } else {
    unsigned time = uptime();
    write_len = snprintf(ch_buf, len, "t %u\n", time);
  }
  // printf("events_read:ch=%c,%d\n", ch_buf[write_len-1], ch_buf[write_len-1]);
  return write_len;
}

// 返回-1表示没有按过F1,F2,F3之一, 否则返回1,2,3之一, 如果有多个功能键, 返回最后一次的
int check_function_key() {
  int key_code;
  int ret = -1;
  // while((key_code = read_key()) != _KEY_NONE) {
    key_code = read_key();
    key_code &= ~0x8000; // 也就是无论是按起还是按下, 都做相同的处理
    switch(key_code) {
      case _KEY_F1: {ret = 1; Log_debug("F1 key"); break;}
      case _KEY_F2: {ret = 2; Log_debug("F2 key"); break;}
      case _KEY_F3: {ret = 3; Log_debug("F3 key"); break;}
      default: {}
    }
  // }
  return ret;
}

char dispinfo[128] __attribute__((used)) = {};
static int height;
static int width;
int screen_size;

// len到底是否包括\0?
size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  // 这里offset参数是起作用的
  // 未对len做检查
  char* ch_buf = (char*)buf;
  memcpy(ch_buf, dispinfo+offset, len);
  ch_buf[len]='\0';
  return len;
}

#define min(a,b) ((a<b)?a:b)
//在navy中的调用, 是: `fwrite(&canvas[i * canvas_w], sizeof(uint32_t), canvas_w, fbdev);` 其实写的是convas的一整行, 也就是说, 不会出现跨行的情况, 但是为了避免错误, 还是处理了跨行的情况
// 这里的offset是*4的, 需要/4
size_t fb_write(const void *buf, size_t bytes_offset, size_t bytes_len) {
  LLog("enter fb_write");
  _yield();
  // void draw_rect(uint32_t *pixels, int x, int y, int w, int h)
  size_t offset = bytes_offset >> 2;
  size_t len = bytes_len >> 2;
  size_t cur_pos = 0;
  size_t remain_len = min(len, screen_size-offset); // 以格子为单位
  // printf("remain_len:%d, len:%d, screen_size:%d, offset:%d\n", remain_len, len, screen_size, offset);
  int y = offset/width;
  int x = offset%width;
  int tmp_write_len = 0; // 这是以4字节为单位的, 或者说像素数
  while(remain_len>0) {
    tmp_write_len = min(width-x, remain_len); 
    draw_rect(buf+cur_pos, x, y, tmp_write_len, 1);
    // printf("cur_pos=%d,x=%d,y=%d,width=%d,height=%d\n", cur_pos, x, y, tmp_write_len, 1);
    // 更新x,y
    ++y;
    x=0;
    // 更新curpos和remain_len
    remain_len -= tmp_write_len; 
    cur_pos += tmp_write_len<<2; // 以字节为单位
  }

  LLog("leave fb_write");
  return cur_pos;
}

size_t fbsync_write(const void *buf, size_t offset, size_t len) {
  (void)buf;
  (void)offset;
  (void)len;
  draw_sync();
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  _ioe_init();

  height = screen_height();
  width = screen_width();
  screen_size = height*width;
// WIDTH:640
// HEIGHT:480
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d", width, height);
  // printf("%s\n", dispinfo);
}

// int get_fb_size() {
//   return screen_size;
// }
