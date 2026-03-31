#include "proc.h"

static struct task* g_current_task = 0;
static struct task* g_first_task = 0;
static struct thread_context g_sched_ctx;

static uint32_t align_down(uint32_t value, uint32_t align) {
   return value & ~(align - 1);
}

static void task_bootstrap(void) {
   g_current_task->entry();
   for(;;)
      ;
}

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
   stack_top = align_down(stack_top, 16);

   task->ctx.ra = (uint32_t)(uintptr_t)task_bootstrap;
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
   task_exit();
}
