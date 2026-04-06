#include "k.h"
#include <stdint.h>
#include "cli.h"
#include "proc.h"
#include "trap.h"

int main(void) {
   g_sched_mode = SCHED_PREE;
   sched_init();
   task_spawn("cli", prog_cli);
   trap_enable();
   sched_start();

   for (;;)
      ;
   return 0;
}
