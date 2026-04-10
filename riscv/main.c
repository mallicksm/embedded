#include "k.h"
#include <stdint.h>
#include "proc.h"

int main(void) {
   ASSERT_INTR_OFF();

   g_sched_mode = SCHED_COOP;

   sched_init();

   trap_init();

   ASSERT_INTR_OFF();

   task_spawn("cli", prog_cli);

   ASSERT_MSG(g_first_task != 0, "main: no runnable tasks\n");

   sched_start(); // does not return

   UNREACHABLE();

   return 0;
}
