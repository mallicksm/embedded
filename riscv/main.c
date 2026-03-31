#include <stdint.h>
#include "cli.h"
#include "proc.h"

static struct task cli_task;
static uint8_t cli_stack[1024];

int main(void) {
   sched_init();
   task_init(&cli_task, "cli", cli, cli_stack, sizeof(cli_stack));
   sched_start();

   for (;;)
      ;
   return 0;
}
