#include "proc.h"

static struct task* g_current_task = 0;
static struct task* g_first_task = 0;

void sched_init(void) {
   g_current_task = 0;
   g_first_task = 0;
}

void task_init(struct task* task,
               const char* name,
               void (*entry)(void),
               uint8_t* stack_base,
               uint32_t stack_size) {
   task->name = name;
   task->ctx.ra = 0;
   task->ctx.sp = 0;

   task->ctx.s0 = 0;
   task->ctx.s1 = 0;
   task->ctx.s2 = 0;
   task->ctx.s3 = 0;
   task->ctx.s4 = 0;
   task->ctx.s5 = 0;
   task->ctx.s6 = 0;
   task->ctx.s7 = 0;
   task->ctx.s8 = 0;
   task->ctx.s9 = 0;
   task->ctx.s10 = 0;
   task->ctx.s11 = 0;

   task->state = TASK_RUNNABLE;
   task->entry = entry;
   task->stack_base = stack_base;
   task->stack_size = stack_size;
   task->next = 0;

   /*
    * Rudimentary first step:
    * we support one task for now, and sched_start()
    * will directly call its entry function.
    */
   if (g_first_task == 0) {
      g_first_task = task;
   }
}

void task_yield(void) {
   /*
    * Stub for now.
    *
    * Later this will:
    *    - mark current task runnable
    *    - hand control to the scheduler
    *    - eventually resume here after thread_switch()
    *
    * For the current intermediate step, yield is a no-op.
    */
}

void task_exit(void) {
   if (g_current_task != 0) {
      g_current_task->state = TASK_DONE;
   }

   /*
    * Rudimentary behavior for now:
    * once a task exits, stop here forever.
    *
    * Later this should transfer control back to the scheduler.
    */
   for (;;)
      ;
}

struct task* task_current(void) {
   return g_current_task;
}

struct task* sched_pick(void) {
   if (g_first_task == 0) {
      return 0;
   }

   if (g_first_task->state != TASK_RUNNABLE) {
      return 0;
   }

   return g_first_task;
}

void sched_start(void) {
   struct task* task = sched_pick();

   if (task == 0) {
      for (;;)
         ;
   }

   g_current_task = task;
   g_current_task->state = TASK_RUNNING;

   /*
    * Intermediate step:
    * directly call the task entry as a normal C function.
    *
    * Later this will become real scheduler entry plus
    * thread_switch() into the task context.
    */
   g_current_task->entry();

   /*
    * If the task entry returns, treat it as task exit.
    */
   task_exit();
}
