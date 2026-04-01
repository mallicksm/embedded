#include <stdint.h>
#include "tasks.h"
#include "printf.h"

static uint8_t a_stack[512];

void a(void) {
   for (;;) {
      printf("a\n");
      task_yield();
   }
}

static struct task task_a = {
   .name = "a",
   .state = TASK_UNUSED,
   .entry = a,
   .stack_base = a_stack,
   .stack_size = sizeof(a_stack),
   .next = 0,
};

REGISTER_TASK("a", task_a);

static uint8_t b_stack[1024];

void b(void) {
   for (;;) {
      printf("b\n");
      task_yield();
   }
}

static struct task task_b = {
   .name = "b",
   .state = TASK_UNUSED,
   .entry = b,
   .stack_base = b_stack,
   .stack_size = sizeof(b_stack),
   .next = 0,
};

REGISTER_TASK("b", task_b);
