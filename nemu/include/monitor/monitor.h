#ifndef __MONITOR_H__
#define __MONITOR_H__

#include "common.h"

enum { NEMU_STOP, NEMU_RUNNING, NEMU_END, NEMU_ABORT };

typedef struct {
  int state;
  vaddr_t halt_pc;
  uint32_t halt_ret;
} NEMUState;

enum { // 似乎框架中的enum都是小写的
  CMD_HELP_IDX,
  CMD_C_IDX,
  CMD_Q_IDX,
  CMD_SI_IDX,
  CMD_X_IDX,
  CMD_INFO_IDX
};

extern NEMUState nemu_state;

#endif
