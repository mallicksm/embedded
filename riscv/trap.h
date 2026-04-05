#ifndef TRAP_H
#define TRAP_H

#include <stdint.h>

typedef enum {
   SCHED_COOP = 0,
   SCHED_PREE = 1,
} sched_mode_t;

void timer_set(uint32_t);
void trap_enable(void);
void trap_handler(void);

#endif
