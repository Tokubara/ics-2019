#include "common.h"
#include <amdev.h>

size_t serial_write(const void *buf, size_t offset, size_t len) {
  (void)offset;
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

size_t events_read(void *buf, size_t offset, size_t len) {
  (void)offset;
  // 读键盘
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
  return write_len;
}

static char dispinfo[128] __attribute__((used)) = {};

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

size_t fbsync_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
}
