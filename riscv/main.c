#include <stdint.h>
#include "cli.h"
#include "proc.h"
#include "trap.h"

int main(void) {
   sched_init();
   task_spawn("cli", run_cli);
   trap_enable();
   sched_start();

   for (;;)
      ;
   return 0;
}
