#include "proc.h"

static struct task* g_current_task = 0;
static struct task* g_first_task = 0;
static struct thread_context g_sched_ctx;

//------------------------------------------------------------------------------
// First-entry trampoline for a never-before-run task.
//
// task_init() builds a brand new saved thread context with:
//    - SP set to the top of the task's stack
//    - RA set to task_trampoline()
//
// The first time the scheduler switches to that task, thread_switch() restores
// the saved context and its final ret lands here. From here, we invoke the
// task's real entry function on the task's own stack.
//
// This function is internal to proc.c because it is part of task startup
// machinery, not public scheduler API.
//------------------------------------------------------------------------------
static void task_trampoline(void) {
   g_current_task->entry();
   for (;;)
      ;
}

_Static_assert(sizeof(struct thread_context) == 56, "ctx size");
_Static_assert(__builtin_offsetof(struct thread_context, ra) == 48, "ra offset");
_Static_assert(__builtin_offsetof(struct thread_context, sp) == 52, "sp offset");

void task_init(struct task* task,
               const char* name,
               void (*entry)(void),
               uint8_t* stack_base,
               uint32_t stack_size) {
   uint32_t stack_top;

   task->name = name;
   task->entry = entry;
   task->stack_base = stack_base;
   task->stack_size = stack_size;
   task->state = TASK_RUNNABLE;
   task->next = 0;

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

   stack_top = (uint32_t)(uintptr_t)(stack_base + stack_size);
   stack_top &= ~(uintptr_t)0xf;

   task->ctx.ra = (uint32_t)(uintptr_t)task_trampoline;
   task->ctx.sp = stack_top;

   if (g_first_task == 0) {
      g_first_task = task;
   }
}

void task_yield(void) {
}

void task_exit(void) {
   if (g_current_task != 0) {
      g_current_task->state = TASK_DONE;
   }

   for (;;)
      ;
}

struct task* task_current(void) {
   return g_current_task;
}

void sched_init(void) {
   g_current_task = 0;
   g_first_task = 0;
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

   thread_switch(&g_sched_ctx, &g_current_task->ctx);

   for (;;)
      ;
}
