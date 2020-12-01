#include "common.h"
int init_monitor(int, char *[]);
void ui_mainloop(int);

void init_regex();
uint8_t make_token(char *);

// 原来的
// int main(int argc, char *argv[]) {
//   /* Initialize the monitor. */
//   int is_batch_mode = init_monitor(argc, argv);

//   /* Receive commands from user. */
//   ui_mainloop(is_batch_mode);

//   return 0;
// }

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  // int is_batch_mode = init_monitor(argc, argv);

  /* Receive commands from user. */
  // ui_mainloop(is_batch_mode);
  init_regex();
  uint32_t len = make_token(argv[1]);
  printf("len=%u\n", len);
  return 0;
}
