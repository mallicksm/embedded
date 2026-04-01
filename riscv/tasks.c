#include "tasks.h"
#include "printf.h"
#include "proc.h"

void a(void) {
   for (;;) {
      printf("a\n");
      task_yield();
   }
}

REGISTER_TASK("a", a);

void b(void) {
   for (;;) {
      printf("b\n");
      task_yield();
   }
}

REGISTER_TASK("b", b);
