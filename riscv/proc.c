#include "k.h"
#include "proc.h"

#define MAX_TASKS 64
#define TASK_STACK_SIZE 1024

static struct task g_task_pool[MAX_TASKS];
static uint8_t g_task_stacks[MAX_TASKS][TASK_STACK_SIZE];
struct task* g_current_task = 0;
struct task* g_first_task = 0;
static struct thread_context g_sched_ctx;

// Protect thread_context to thread_switch.S placement
_Static_assert(sizeof(struct thread_context) == 56, "ctx size");
_Static_assert(__builtin_offsetof(struct thread_context, ra) == 48, "ra offset");
_Static_assert(__builtin_offsetof(struct thread_context, sp) == 52, "sp offset");

// Forward declaration of local statics
static void task_list_add(struct task* task);
static void task_list_remove(struct task* task);
static struct task* task_list_pick(void);

//------------------------------------------------------------------------------
// Initialize a new task (never run before)
//
// Called by: task_start()
//
// What it does:
//    - Sets up the Task Control Block (TCB)
//    - Assigns stack memory and initializes stack pointer (sp)
//    - Stores entry function (task entry point)
//    - Prepares initial CPU context:
//         * ra → task_trampoline()   (what executes when entry returns)
//         * sp → top of task stack
//    - Marks task as RUNNABLE
//
// Control-flow model:
//    This does NOT start execution.
//
//    The task is inserted into the scheduler queue. On first schedule:
//       thread_switch() restores this context →
//       execution begins at entry() with a valid stack.
//
//    If entry() ever returns:
//       control flows into task_trampoline() (never back to scheduler directly)
//
// Why trampoline exists:
//    Prevents falling off the stack and gives a controlled place
//    to handle task exit (cleanup, loop, or panic).
//------------------------------------------------------------------------------
static void task_trampoline(void);
static void task_init(struct task* task,
               const char* name,
               void (*entry)(void),
               uint8_t* stack_base,
               uint32_t stack_size) {
   uint32_t stack_top;

   ASSERT_MSG(task != 0, "task_init: null task\n");
   ASSERT_MSG(name != 0, "task_init: null name\n");
   ASSERT_MSG(entry != 0, "task_init: null entry\n");
   ASSERT_MSG(stack_base != 0, "task_init: null stack\n");

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
   ASSERT_MSG(g_current_task != 0, "trampoline: no current task\n");

   g_current_task->entry();
   task_exit();

   UNREACHABLE();
}

//------------------------------------------------------------------------------
// public
//------------------------------------------------------------------------------
//    task_start()
//       Establish one task and make it runnable.
//       This prepares the task control block, initializes its saved execution
//       context, records its stack and entry point, and adds it to the task
//       pool so the scheduler may later run it.
//
void task_spawn(const char* name, void (*entry)(void)) {
   int i;
   struct task* slot = 0;

   ASSERT_MSG(name != 0, "task_start: null name\n");
   ASSERT_MSG(entry != 0, "task_start: null entry\n");

   for (i = 0; i < MAX_TASKS; i++) {
      if (g_task_pool[i].state == TASK_UNUSED) {
         slot = &g_task_pool[i];
         break;
      }
   }

   ASSERT_MSG(slot != 0, "task_start: no free task slots\n");

   task_init(slot,
             name,
             entry,
             g_task_stacks[i],
             TASK_STACK_SIZE);

   task_list_add(slot);
}

//------------------------------------------------------------------------------
// public
//------------------------------------------------------------------------------
// Yield the CPU voluntarily from the current task.
//
// Called by: the currently running task.
//
// What it does:
//    If another runnable task exists:
//       - Saves the current task's CPU context
//       - Selects the next runnable task
//       - Switches execution to that task
//
//    If no other runnable task exists, or if the scheduler selects the
//    current task again, this function returns immediately.
//
// Control-flow story:
//    From the task's point of view, execution resumes at the instruction
//    immediately following task_yield() when this task is scheduled again.
//
//    Although task_yield() may transfer control to other tasks, the calling
//    task observes it as a simple function call that eventually returns.
//------------------------------------------------------------------------------
void task_yield(void) {
   struct task* current;
   struct task* next;

   current = g_current_task;
   ASSERT_MSG(current != 0, "yield: no current task\n");

   next = task_list_pick();
   ASSERT_MSG(next != 0, "yield: no runnable task\n");

   if (next == current) {
      return;
   }

   // only downgrade if still running
   if (current->state == TASK_RUNNING) {
      current->state = TASK_RUNNABLE;
   }

   next->state = TASK_RUNNING;
   g_current_task = next;

   thread_switch(&current->ctx, &next->ctx);

   // resumed here
   g_current_task = current;
   current->state = TASK_RUNNING;
}

//------------------------------------------------------------------------------
// public
//------------------------------------------------------------------------------
// Terminate the current task.
//
// Called by:
//    - The currently running task (explicit exit)
//    - task_trampoline() after the task entry function returns
//
// What it does:
//    - Selects the next runnable task
//    - Removes the current task from the runnable list
//    - Marks it UNUSED
//    - Switches execution to the selected task
//
// System invariant:
//    At least one task (idle) is always RUNNABLE
//    If no runnable task is found, the system halts.
//
// Control-flow story:
//    The current task never resumes execution after calling task_exit().
//------------------------------------------------------------------------------
void task_exit(void) {
   struct task* dead = g_current_task;

   ASSERT_MSG(dead != 0, "exit: no current task\n");

   // Pick next BEFORE removing self
   struct task* next = task_list_pick();

   task_list_remove(dead);
   dead->state = TASK_UNUSED;

   // At least one task (idle) is always RUNNABLE
   ASSERT_MSG(next != 0, "exit: no runnable task\n");

   g_current_task = next;
   next->state = TASK_RUNNING;

   thread_switch(&dead->ctx, &next->ctx);

   UNREACHABLE();
}

void task_sleep(int ticks) {
   ASSERT_MSG(g_current_task != 0, "sleep: no current task\n");

   g_current_task->sleep_ticks = ticks;
   g_current_task->state = TASK_SLEEPING;

   task_yield();
}

int task_getpid(void) {
   ASSERT_MSG(g_current_task != 0, "getpid: no current task\n");
   return g_current_task - g_task_pool;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static struct task* task_by_pid(int);
int task_kill(int pid) {
   struct task* t;

   t = task_by_pid(pid);
   if (t == 0) {
      return -1;
   }

   // do not allow killing idle (pid 0 typically)
   ASSERT_MSG(t != 0, "kill: null task\n");

   // self-kill → reuse existing path
   if (t == g_current_task) {
      task_exit();
      UNREACHABLE();
   }

   // remove from scheduler
   task_list_remove(t);
   t->state = TASK_UNUSED;

   return 0;
}
static struct task* task_by_pid(int pid) {
   if (pid < 0 || pid >= MAX_TASKS) {
      return 0;
   }

   if (g_task_pool[pid].state == TASK_UNUSED) {
      return 0;
   }

   return &g_task_pool[pid];
}

//------------------------------------------------------------------------------
// public
//------------------------------------------------------------------------------
// Scheduler and task control methods
//
//    sched_init()
//       Initialize the scheduling subsystem and reset internal task-pool state.
//       This should be called once during boot before any tasks are started.
//
static void idle(void);
void sched_init(void) {
   g_current_task = 0;
   g_first_task = 0;

   for (int i = 0; i < MAX_TASKS; i++) {
      g_task_pool[i].state = TASK_UNUSED;
      g_task_pool[i].next = 0;
   }

   task_spawn("idle", idle);
}

static void idle(void) {
   while (1) {
      task_yield();
   }
}

//------------------------------------------------------------------------------
// public
//------------------------------------------------------------------------------
//    sched_start()
//       Enter scheduled execution for the first time.
//       This should be called once after at least one task has been started.
//       It picks the first runnable task and transfers control to it.
//------------------------------------------------------------------------------
void sched_start(void) {
   struct task* task = task_list_pick();

   ASSERT_MSG(task != 0, "sched_start: no runnable task\n");

   task->state = TASK_RUNNING;
   g_current_task = task;

   thread_switch(&g_sched_ctx, &task->ctx);

   UNREACHABLE();
}

void sched_tick(void) {
   struct task* t;

   if (g_first_task == 0) {
      return;
   }

   t = g_first_task;
   do {
      if (t->state == TASK_SLEEPING) {
         t->sleep_ticks--;
         if (t->sleep_ticks <= 0) {
            t->sleep_ticks = 0;
            t->state = TASK_RUNNABLE;
         }
      }
      t = t->next;
   } while (t != g_first_task);
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

   start = (g_current_task != 0) ? g_current_task->next : g_first_task;
   ASSERT_MSG(start != 0, "pick: broken task list\n");

   t = start;
   do {
      if (t->state == TASK_RUNNABLE)
         return t;
      t = t->next;
   } while (t != start);

   return 0;
}

static void task_list_add(struct task* task) {
   struct task* p;

   ASSERT_MSG(task != 0, "add: null task\n");

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

   ASSERT_MSG(task != 0, "remove: null task\n");
   ASSERT_MSG(g_first_task != 0, "remove: empty list\n");

   if (g_first_task == task && g_first_task->next == g_first_task) {
      g_first_task = 0;
      task->next = 0;
      return;
   }

   p = g_first_task;
   while (p->next != task && p->next != g_first_task) {
      p = p->next;
   }

   ASSERT_MSG(p->next == task, "remove: task not in list\n");

   p->next = task->next;

   if (g_first_task == task) {
      g_first_task = task->next;
   }

   task->next = 0;
}
