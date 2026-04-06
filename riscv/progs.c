#include "k.h"
#include "progs.h"
#include "printf.h"
#include "proc.h"

DEFINE_PROG(a, PROG_COOP);
DEFINE_PROG(b, PROG_COOP);
DEFINE_PROG(i, PROG_PREE);
DEFINE_PROG(j, PROG_PREE);

void prog_slow(void) {
   while (1) {
      for (volatile int i = 0; i < 100000000; i++)
         ;
      printf("L%d", task_getpid());
   }
}
REGISTER_PROG("slow", prog_slow);

void prog_sleeper(void) {
   while (1) {
      printf("S%d", task_getpid());
      task_sleep(50);
   }
}
REGISTER_PROG("sleeper", prog_sleeper);

void run_worker() {
   printf("A\n");
   task_sleep(3);
   printf("B\n");

   while (1)
      task_yield();
}
REGISTER_PROG("worker", run_worker);
