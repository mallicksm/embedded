#ifndef TRAP_H
#define TRAP_H

#include <stdint.h>

void timer_set(uint32_t delta);
void trap_enable(void);
void trap_handler(void);

#endif
