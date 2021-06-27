#include <am.h>
#include <amdev.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

size_t __am_input_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_INPUT_KBD: {
      _DEV_INPUT_KBD_t *kbd = (_DEV_INPUT_KBD_t *)buf;
      unsigned int nemu_keycode = inl(KBD_ADDR); // 这是包含0x8000的
      // 没有单独处理_KEY_NONE, 因为下面对它也适用, 反正是0
      kbd->keydown = ((nemu_keycode & 0x8000)>0);
      kbd->keycode = (nemu_keycode & ~0x8000);
      return sizeof(_DEV_INPUT_KBD_t);
    }
  }
  return 0;
}
