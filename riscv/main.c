#include "k.h"
#include <stdint.h>
#include "proc.h"

int main(void) {
   g_sched_mode = SCHED_COOP;
   sched_init();
   task_spawn("cli", prog_cli);
   trap_enable();
   sched_start();

   for (;;)
      ;
   return 0;
}
