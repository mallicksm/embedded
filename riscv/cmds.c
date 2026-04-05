#include <stdint.h>
#include "uart.h"
#include "cmds.h"
#include "printf.h"
#include "stdlib.h"
#include "tasks.h"
#include "string.h"
#include "proc.h"
#include "trap.h"

static int cmd_help(int argc, char** argv) {
   const struct cmds* cmd = __cmds_start;

   (void)argc;
   (void)argv;

   uart_puts("Commands:\n");
   while (cmd < __cmds_end) {
      uart_puts("  ");
      uart_puts(cmd->name);
      uart_puts("\n");
      cmd++;
   }

   return 0;
}

REGISTER_CMD("help", cmd_help);

static int cmd_echo(int argc, char** argv) {
   int i;

   for (i = 1; i < argc; i++) {
      uart_puts(argv[i]);
      if (i + 1 < argc) {
         uart_putc(' ');
      }
   }
   uart_puts("\n");

   return 0;
}

REGISTER_CMD("echo", cmd_echo);

static int cmd_mrd(int argc, char** argv) {
   if (argc < 2) {
      uart_puts("Usage: mrd <addr> [count]\n");
      return -1;
   }

   uint32_t addr = strtoul(argv[1], 0, 0);
   uint32_t count = (argc >= 3) ? strtoul(argv[2], 0, 0) : 1;

   volatile uint32_t* p = (uint32_t*)addr;

   for (uint32_t i = 0; i < count; i++) {
      printf("%p: 0x%08x\n", (void*)&p[i], p[i]);
   }

   return 0;
}

REGISTER_CMD("mrd", cmd_mrd);

static int cmd_mwr(int argc, char** argv) {
   if (argc < 3) {
      uart_puts("Usage: mwr <addr> <data> [count]\n");
      return -1;
   }

   uint32_t addr = strtoul(argv[1], 0, 0);
   uint32_t data = strtoul(argv[2], 0, 0);
   uint32_t count = (argc >= 4) ? strtoul(argv[3], 0, 0) : 1;

   volatile uint32_t* p = (uint32_t*)addr;

   for (uint32_t i = 0; i < count; i++) {
      p[i] = data;
      printf("%p: 0x%08x\n", (void*)&p[i], data);
   }

   return 0;
}

REGISTER_CMD("mwr", cmd_mwr);

int cmd_run(int argc, char** argv) {
   const struct tasks* task;

   if (argc != 2) {
      printf("usage: run <task>\n");
      return -1;
   }

   task = __tasks_start;
   while (task < __tasks_end) {
      if (strcmp(task->name, argv[1]) == 0) {
         task_spawn(task->name, task->fn);
         return 0;
      }
      task++;
   }

   printf("Unknown task: %s\n", argv[1]);
   return -1;
}

REGISTER_CMD("run", cmd_run);

extern struct task* g_first_task;
int cmd_ps(int argc, char** argv) {
   (void)argc;
   (void)argv;
   struct task* t = g_first_task;

   printf("NAME\tSTATE   \tSLEEP\n");

   do {
      printf("%s\t", t->name);

      if (t->state == TASK_RUNNING)
         printf("RUNNING \t");
      else if (t->state == TASK_RUNNABLE)
         printf("RUNNABLE\t");
      else if (t->state == TASK_SLEEPING)
         printf("SLEEPING\t");

      printf("%d\n", t->sleep_ticks);

      t = t->next;
   } while (t != g_first_task);
   return 0;
}

REGISTER_CMD("ps", cmd_ps);

/* cmds.c */
static int cmd_tick(int argc, char** argv) {
   (void)argc;
   (void)argv;
   sched_tick();
   return 0;
}

REGISTER_CMD("tick", cmd_tick);

volatile sched_mode_t g_sched_mode = SCHED_COOP;

static int cmd_sched(int argc, char** argv) {
   if (argc == 1) {
      if (g_sched_mode == SCHED_COOP)
         printf("Current: coop\n");
      else if (g_sched_mode == SCHED_PREE)
         printf("Current: pree");
      else
         return 0;
   } else if (argc == 2) {
      if (!strcmp(argv[1], "coop")) {
         g_sched_mode = SCHED_COOP;
         printf("Active: coop");
      } else if (!strcmp(argv[1], "pree")) {
         g_sched_mode = SCHED_PREE;
         printf("Active: pree");
      } else {
         return 0;
      }
   } else {
      return 0;
   }
   return 0;
}

REGISTER_CMD("sched", cmd_sched);
