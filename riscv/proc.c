#include "proc.h"

#define MAX_TASKS 64
#define TASK_STACK_SIZE 1024

static struct task g_task_pool[MAX_TASKS];
static uint8_t g_task_stacks[MAX_TASKS][TASK_STACK_SIZE];
static struct task* g_current_task = 0;
static struct task* g_first_task = 0;
static struct thread_context g_sched_ctx;

_Static_assert(sizeof(struct thread_context) == 56, "ctx size");
_Static_assert(__builtin_offsetof(struct thread_context, ra) == 48, "ra offset");
_Static_assert(__builtin_offsetof(struct thread_context, sp) == 52, "sp offset");

static void task_list_add(struct task* task);
static void task_list_remove(struct task* task);
static struct task* task_list_pick(void);

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
static void task_trampoline(void);
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
}

static void task_trampoline(void) {
   g_current_task->entry();
   task_exit();
   for (;;)
      ;
}

//------------------------------------------------------------------------------
// Voluntarily yield the CPU from the current task.
//
// If there is another runnable task, save the current task's execution
// context, switch to the selected task, and resume execution there.
//
// If no other runnable task exists, or if the scheduler selects the
// current task again, this function returns immediately and execution
// continues in the current task.
//
// When the current task is later scheduled again, execution resumes
// immediately after the thread_switch() inside this function, and
// task_yield() then returns to its caller as a normal function call.
//------------------------------------------------------------------------------
void task_yield(void) {
   struct task* current;
   struct task* next;

   current = g_current_task;
   if (current == 0) {
      return;
   }

   next = task_list_pick();
   if (next == 0 || next == current) {
      return;
   }

   current->state = TASK_RUNNABLE;
   next->state = TASK_RUNNING;
   g_current_task = next;

   thread_switch(&current->ctx, &next->ctx);

   g_current_task = current;
   current->state = TASK_RUNNING;
}

void task_exit(void) {
   struct task* dead = g_current_task;
   struct task* next;

   if (dead == 0) {
      for (;;)
         ;
   }

   task_list_remove(dead);
   dead->state = TASK_UNUSED;

   if (g_first_task == 0) {
      g_current_task = 0;
      thread_switch(&dead->ctx, &g_sched_ctx);

      for (;;)
         ;
   }

   next = g_first_task;
   g_current_task = next;
   next->state = TASK_RUNNING;

   thread_switch(&dead->ctx, &next->ctx);

   for (;;)
      ;
}

void sched_init(void) {
   g_current_task = 0;
   g_first_task = 0;
   for (int i = 0; i < MAX_TASKS; i++) {
      g_task_pool[i].state = TASK_UNUSED;
      g_task_pool[i].next = 0;
   }
}

void sched_start(void) {
   struct task* task = task_list_pick();

   if (task == 0) {
      for (;;)
         ;
   }

   task->state = TASK_RUNNING;
   g_current_task = task;

   thread_switch(&g_sched_ctx, &task->ctx);

   for (;;)
      ;
}

void task_start(const char* name, void (*entry)(void)) {
   int i;
   struct task* slot = 0;

   if (name == 0 || entry == 0) {
      return;
   }

   for (i = 0; i < MAX_TASKS; i++) {
      if (g_task_pool[i].state == TASK_UNUSED) {
         slot = &g_task_pool[i];
         break;
      }
   }

   if (slot == 0) {
      return;
   }

   task_init(slot,
             name,
             entry,
             g_task_stacks[i],
             TASK_STACK_SIZE);

   task_list_add(slot);
}

//------------------------------------------------------------------------------
// Task pool methods
//
// These static helper functions operate on the internal task pool stored as
// a circular singly-linked list rooted at g_first_task.
//
//    task_list_pick()
//       Pick the next TASK_RUNNABLE task from the task pool.
//       This should be called by scheduler-facing code that needs to choose
//       the next task to run, such as sched_start(), task_yield(), and later
//       task_exit().
//
//    task_list_add()
//       Add one task to the task pool.
//       This should be called by task creation / initialization code when a
//       task is made known to the scheduler, such as task_init().
//
//    task_list_remove()
//       Remove one task from the task pool.
//       This should be called by task teardown or state-transition code when
//       a task must no longer participate in scheduling, such as task_exit()
//       or later blocking/destruction paths.
//
// These functions are internal to proc.c and are not part of the public API.
//------------------------------------------------------------------------------
static struct task* task_list_pick(void) {
   struct task* start;
   struct task* t;

   if (g_first_task == 0)
      return 0;

   /*
    * Circular ring: task->next (single-task ring: next points to self).
    * Start after current when there is one; otherwise from list head.
    * Return the first TASK_RUNNABLE found; skip TASK_RUNNING / others.
    */
   start = (g_current_task != 0) ? g_current_task->next : g_first_task;
   if (start == 0)
      return 0;

   t = start;
   do {
      if (t->state == TASK_RUNNABLE)
         return t;
      t = t->next;
   } while (t != start && t != 0);

   return 0;
}

static void task_list_add(struct task* task) {
   struct task* p;

   if (g_first_task == 0) {
      g_first_task = task;
      task->next = task;
      return;
   }

   p = g_first_task;
   while (p->next != g_first_task) {
      p = p->next;
   }

   p->next = task;
   task->next = g_first_task;
}

static void task_list_remove(struct task* task) {
   struct task* p;

   if (g_first_task == 0 || task == 0) {
      return;
   }

   if (g_first_task == task && g_first_task->next == g_first_task) {
      g_first_task = 0;
      task->next = 0;
      return;
   }

   p = g_first_task;
   while (p->next != task && p->next != g_first_task) {
      p = p->next;
   }

   if (p->next != task) {
      return;
   }

   p->next = task->next;

   if (g_first_task == task) {
      g_first_task = task->next;
   }

   task->next = 0;
}

