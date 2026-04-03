#include "tasks.h"
#include "printf.h"
#include "proc.h"

void run_a(void) {
   for (;;) {
      printf("Task:a\n");
      task_yield();
   }
}

REGISTER_TASK("a", run_a);

void run_b(void) {
   for (;;) {
      printf("Task:b\n");
      task_yield();
   }
}

REGISTER_TASK("b", run_b);

void run_c(void) {
   for (int i = 0; i < 100000; i++) {
      printf("Task:c\n");
      task_yield();
   }
}

REGISTER_TASK("c", run_c);
