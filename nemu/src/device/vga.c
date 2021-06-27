#include "common.h"

#ifdef HAS_IOE

#include "device/map.h"
#include <SDL2/SDL.h>

#define VMEM 0xa0000000

#define SCREEN_PORT 0x100 // Note that this is not the standard
#define SCREEN_MMIO 0xa1000100
#define SYNC_PORT 0x104 // Note that this is not the standard
#define SYNC_MMIO 0xa1000104
#define SCREEN_H 300
#define SCREEN_W 400

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

static uint32_t (*vmem) [SCREEN_W] = NULL;
static uint32_t *screensize_port_base = NULL;

static inline void update_screen() {
  SDL_UpdateTexture(texture, NULL, vmem, SCREEN_W * sizeof(vmem[0][0]));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

static void vga_io_handler(uint32_t offset, int len, bool is_write) {
  if(is_write) { // 其实我想assert(is_write), 对应am中__am_video_write的sync情况
    update_screen();
  }
}

void init_vga() {
  char title[128];
  sprintf(title, "%s-NEMU", str(__ISA__));

  SDL_Init(SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(SCREEN_W * 2, SCREEN_H * 2, 0, &window, &renderer); //? 为什么这里的宽和高要*2
  SDL_SetWindowTitle(window, title);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
      SDL_TEXTUREACCESS_STATIC, SCREEN_W, SCREEN_H);

  screensize_port_base = (void *)new_space(8); // 它本身的类型是i32, 因此有2个i32 
  screensize_port_base[0] = ((SCREEN_W) << 16) | (SCREEN_H); // 也就是说screensize_port_base[0]这32位中, 高16位是宽度, 低16位是高度. 我怀疑这不是SDL的一部分, 只是nemu的约定
  add_pio_map("screen", SCREEN_PORT, (void *)screensize_port_base, 8, vga_io_handler); // 那边读取宽度和高度, 也会调用vga_io_handler, 这时候vga_io_handler应该不做处理 // 我怀疑框架这里写错了, screen就应该是读取宽度和高度的, 我不觉得它应该有同步寄存器
  add_mmio_map("screen", SCREEN_MMIO, (void *)screensize_port_base, 8, vga_io_handler);

  vmem = (void *)new_space(0x80000);
  add_mmio_map("vmem", VMEM, (void *)vmem, 0x80000, NULL);
}
#endif	/* HAS_IOE */
