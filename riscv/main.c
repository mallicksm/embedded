#include <stdint.h>
#include "cli.h"
#include "proc.h"

int main(void) {
   sched_init();
   task_cli();
   sched_start();

   for (;;)
      ;
   return 0;
}
