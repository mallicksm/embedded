#include "tasks.h"
#include "printf.h"
#include "proc.h"

DEFINE_TASK(a, TASK_COOP);
DEFINE_TASK(b, TASK_COOP);
DEFINE_TASK(i, TASK_PREE);
DEFINE_TASK(j, TASK_PREE);
void run_slow(void) {
   while (1) {
      for (volatile int i = 0; i < 10000000; i++)
         ;
      printf("L%d", task_getpid());
   }
}
REGISTER_TASK("slow", run_slow);

void run_sleeper(void) {
   while (1) {
      printf("S,%d", task_getpid());
      task_sleep(50);
   }
}
REGISTER_TASK("sleeper", run_sleeper);

void run_worker() {
   printf("A\n");
   task_sleep(3);
   printf("B\n");

   while (1)
      task_yield();
}
REGISTER_TASK("worker", run_worker);
