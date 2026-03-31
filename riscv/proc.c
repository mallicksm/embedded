#include <stdint.h>

#include "proc.h"

struct task* g_current_task;

static struct task* g_run_head;

static void task_ring_append(struct task* task) {
   if (g_run_head == 0) {
      g_run_head = task;
      task->next = task;
      return;
   }

   struct task* tail = g_run_head;
   while (tail->next != g_run_head)
      tail = tail->next;

   tail->next = task;
   task->next = g_run_head;
}

static struct task* pick_next(void) {
   if (g_run_head == 0 || g_current_task == 0)
      return 0;

   struct task* t = g_current_task->next;
   struct task* start = t;

   do {
      if (t->state == TASK_RUNNABLE || t->state == TASK_RUNNING)
         return t;
      t = t->next;
   } while (t != start);

   return 0;
}

void sched_init(void) {
   g_current_task = 0;
   g_run_head = 0;
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

   if (stack_base != 0 && stack_size >= 64) {
      uintptr_t top = (uintptr_t)(stack_base + stack_size);
      top &= ~(uintptr_t)15;
      top -= 16;
      task->ctx.sp = (uint32_t)top;
      /*
       * First resume: ret in thread_switch lands in entry() with ctx.ra = entry.
       * No trampoline — ra is not a normal return address on that first entry.
       * After task_yield, ctx matches a normal saved frame. If entry() returns,
       * behavior is undefined unless you add a trampoline (or set ra to task_exit).
       */
      task->ctx.ra = (uint32_t)(uintptr_t)entry;
   }

   task_ring_append(task);
}

void task_yield(void) {
   struct task* from = g_current_task;
   struct task* to = pick_next();

   if (to == 0 || to == from)
      return;

   from->state = TASK_RUNNABLE;
   to->state = TASK_RUNNING;
   g_current_task = to;

   thread_switch(&from->ctx, &to->ctx);
}

void task_exit(void) {
   struct task* from = g_current_task;

   if (from != 0)
      from->state = TASK_DONE;

   struct task* to = pick_next();

   if (to == 0 || to == from) {
      for (;;) {
      }
   }

   to->state = TASK_RUNNING;
   g_current_task = to;
   thread_switch(&from->ctx, &to->ctx);

   for (;;) {
   }
}

struct task* task_current(void) {
   return g_current_task;
}

struct task* sched_pick(void) {
   if (g_run_head == 0)
      return 0;

   struct task* t = g_run_head;
   struct task* start = t;

   do {
      if (t->state == TASK_RUNNABLE || t->state == TASK_RUNNING)
         return t;
      t = t->next;
   } while (t != start);

   return 0;
}

void sched_start(void) {
   struct task* to = sched_pick();

   if (to == 0) {
      for (;;) {
      }
   }

   g_current_task = to;
   g_current_task->state = TASK_RUNNING;

   thread_switch(0, &to->ctx);
}
