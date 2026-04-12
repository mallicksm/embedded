#include "k.h"
#include <stdint.h>
#include "proc.h"
#include "cli_exec.h"

//------------------------------------------------------------------------------
// cmds
//------------------------------------------------------------------------------
static int cmd_help(int argc, char** argv) {
   const struct cmds* cmd = __cmds_start;
   const struct progs* prog = __progs_start;

   (void)argc;
   (void)argv;

   printf("Commands:\n");
   while (cmd < __cmds_end) {
      printf("  (c) %s\n", cmd->name);
      cmd++;
   }

   printf("Progs:\n");
   while (prog < __progs_end) {
      printf("  (p) %s\n", prog->name);
      prog++;
   }

   return 0;
}

REGISTER_CMD("help", cmd_help);

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

extern struct task* g_first_task;
int cmd_ps(int argc, char** argv) {
   (void)argc;
   (void)argv;
   struct task* t = g_first_task;

   printf("PID \tNAME\tSTATE   \tSLEEP\n");

   do {
      printf("%d\t%s\t", t->pid, t->name);

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

static int cmd_sched(int argc, char** argv) {
   ASSERT_MSG(argc > 0, "sched: bad argc\n");

   if (argc == 1) {
      if (g_sched_mode == SCHED_COOP) {
         printf("Current: coop\n");
      } else if (g_sched_mode == SCHED_PREE) {
         printf("Current: pree\n");
      } else {
         ASSERT_MSG(0, "sched: invalid mode\n");
      }
      return 0;
   }

   if (argc == 2) {
      if (!strcmp(argv[1], "coop")) {
         g_sched_mode = SCHED_COOP;
         printf("Active: coop\n");
         return 0;
      }

      if (!strcmp(argv[1], "pree")) {
         if (!intr_get()) {
            printf("ERROR: interrupts are OFF (run intr_on first)\n");
            return -1;
         }

         g_sched_mode = SCHED_PREE;
         printf("Active: pree\n");
         return 0;
      }

      printf("Unknown mode: %s\n", argv[1]);
      return 0;
   }

   printf("Usage: sched [coop|pree]\n");
   return 0;
}
REGISTER_CMD("sched", cmd_sched);

static int cmd_kill(int argc, char** argv) {
   int pid;

   if (argc != 2) {
      printf("Usage: kill <pid>\n");
      return 0;
   }

   pid = atoi(argv[1]);

   if (task_kill(pid) < 0) {
      printf("kill: invalid pid %d\n", pid);
      return 0;
   }

   printf("killed %d\n", pid);
   return 0;
}
REGISTER_CMD("kill", cmd_kill);

int cmd_intr(int argc, char** argv) {
   if (argc == 1) {
      // status
      printf("interrupts: %s\n", intr_get() ? "ON" : "OFF");
      return 0;
   }

   if (argc == 2) {
      if (strcmp(argv[1], "on") == 0) {
         intr_on();
         printf("interrupts ON\n");
         return 0;
      }

      if (strcmp(argv[1], "off") == 0) {
         intr_off();
         printf("interrupts OFF\n");
         return 0;
      }
   }

   printf("usage: intr [on|off]\n");
   return -1;
}
REGISTER_CMD("intr", cmd_intr);

//------------------------------------------------------------------------------
// progs
//------------------------------------------------------------------------------
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
