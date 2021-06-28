#include <am.h>
#include <amdev.h>
#include <nemu.h>
#define W 400
#define H 300

size_t __am_video_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_INFO: {
      _DEV_VIDEO_INFO_t *info = (_DEV_VIDEO_INFO_t *)buf;
      unsigned int nemu_wh = inl(SCREEN_ADDR); // 高16位是宽, 低16位是高
      info->width = nemu_wh >> 16;
      info->height = nemu_wh & 0xffff;
      return sizeof(_DEV_VIDEO_INFO_t);
    }
  }
  return 0;
}

static inline int min(int x, int y) {
  return (x < y) ? x : y;
}

extern void* memcpy(void* dst, const void* src, size_t n);
size_t __am_video_write(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_FBCTL: {
      // printf("_DEVREG_VIDEO_FBCTL is called\n");
      _DEV_VIDEO_FBCTL_t *ctl = (_DEV_VIDEO_FBCTL_t *)buf;
      if(ctl->pixels!=NULL) {
        int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
        uint32_t *pixels = ctl->pixels;
        int cp_bytes = sizeof(uint32_t) * min(w, W - x);
        uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
        for (int j = 0; j < h && j + y < H; j++) {
          memcpy(&fb[(y + j) * W + x], pixels, cp_bytes);
          pixels += w;
        }
      }

      if (ctl->sync) {
        // printf("video sync\n");
        outl(SYNC_ADDR, 0); // 写入是几, 不重要, 反正都是调用callback, 不关心写的是几
      }
      return size;
    }
    default: {_halt(1);}
  }
  return 0;
}

void __am_vga_init() {
  int i;
  int size = screen_width() * screen_height();
  // printf("size=%d\n", size);
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (i = 0; i < size; i ++) fb[i] = i;
  draw_sync();
}
