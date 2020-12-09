#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__
#define EXPR_LEN 60 // #todo#

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char* expr;
  uint32_t val;
} WP;

WP *new_wp(char *expr_str);
void free_wp(WP *wp_prev);
void free_wp();
WP *new_wp(char *expr_str);
void print_wps();
void del_wp_NO(int NO);
bool eval_wps();

#endif
