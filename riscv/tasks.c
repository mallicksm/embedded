#include "tasks.h"
#include "printf.h"
#include "proc.h"

void a(void) {
   for (;;) {
      printf("Task:a\n");
      task_yield();
   }
}

REGISTER_TASK("a", a);

void b(void) {
   for (;;) {
      printf("Task:b\n");
      task_yield();
   }
}

REGISTER_TASK("b", b);
