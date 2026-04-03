#pragma once
#include <stdint.h>

//------------------------------------------------------------------------------
// Saved machine context for a suspended task.
//
// Used by:
//    thread_switch(), which saves the old task's preserved register state here
//    and restores the next task's preserved register state from here.
//
// What it contains:
//    Only the low-level register state needed to stop one task and later
//    continue it.
//
//    For RV32, this means:
//       - s0-s11 : callee-saved registers
//       - ra     : return address
//       - sp     : stack pointer
//
// Control-flow story:
//    This is not a full trap frame and not "all CPU state".
//    It is only the continuation state needed so that a later ret can resume
//    the task at the right place, on the right stack.
//------------------------------------------------------------------------------
struct thread_context {
   uint32_t s0;
   uint32_t s1;
   uint32_t s2;
   uint32_t s3;
   uint32_t s4;
   uint32_t s5;
   uint32_t s6;
   uint32_t s7;
   uint32_t s8;
   uint32_t s9;
   uint32_t s10;
   uint32_t s11;

   uint32_t ra;
   uint32_t sp;
};

//------------------------------------------------------------------------------
// Scheduler-visible state of a task.
//
// Used by:
//    the scheduler to decide whether a task may run now, later, or never again.
//
// Meaning of each state:
//    TASK_UNUSED:   Task slot is free and not in use.
//    TASK_RUNNABLE: Task is eligible to be chosen by the scheduler.
//    TASK_RUNNING:  Task is the one currently executing.
//    TASK_BLOCKED:  Task exists but cannot run until some event wakes it.
//    TASK_DONE:     Task has finished and should not be scheduled again.
//------------------------------------------------------------------------------
enum task_state {
   TASK_UNUSED = 0,
   TASK_RUNNABLE,
   TASK_RUNNING,
   TASK_SLEEPING,
   TASK_BLOCKED,
   TASK_DONE,
};

//------------------------------------------------------------------------------
// Task control block.
//
// Used by:
//    the scheduler as the main bookkeeping object for one schedulable task.
//
// What it contains:
//    name:        Human-readable name for debug / diagnostics.
//    ctx:         Saved machine context used by thread_switch().
//    state:       Scheduler-visible run state.
//    sleep_ticks: ticks the task will be sleeping for
//    entry:       Task entry function. This is what the first-entry trampoline invokes the first time the task is scheduled.
//    stack_base:  Stack Base Memory owned by this task.
//    stack_size:  Stack Memory Size owned by this task.
//    next:        Link field for task list / runnable queue management.
//
// Control-flow story:
//    A task is a scheduler-owned object.
//    thread_switch() uses task->ctx.
//------------------------------------------------------------------------------
struct task {
   const char* name;
   struct thread_context ctx;
   enum task_state state;
   int sleep_ticks;
   void (*entry)(void);
   uint8_t* stack_base;
   uint32_t stack_size;
   struct task* next;
};

//------------------------------------------------------------------------------
// Low-level context switch primitive implemented by thread_switch.S.
//
// Called by: scheduler/task-switching code,
// 	1. task_yield()
//      2. task_exit()
//      3. sched_start()
//
// What it does: Exchanges the callee-saved registers
//      1. Saves currently running tasks registers to *old
//      2. Restores next picked tasks registers from *new
//
//    then resumes execution in that restored context.
//
// Control-flow story:
//    thread_switch() does not return in the usual sense. Its final ret
//    continues execution using the restored context from *new.
//
//    - If *new belongs to a task that has run before, execution resumes
//      where that task last yielded completing the function call/return contract.
//    - If *new belongs to a never-before-run task, execution begins at the
//      first-entry trampoline installed by task_init().
//    - schematic below shows the call/return contract.
//      task_one()
//         thread_switch()
//      task_two()
//         thread_switch()
//      task_three()
//         thread_switch()  -> back to task_one()
//
//------------------------------------------------------------------------------
void thread_switch(struct thread_context* old,
                   struct thread_context* new);

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
void task_init(struct task* task,
               const char* name,
               void (*entry)(void),
               uint8_t* stack_base,
               uint32_t stack_size);

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
void task_yield(void);

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
//    At least one runnable task (CLI) must always exist.
//    If no runnable task is found, the system halts.
//
// Control-flow story:
//    The current task never resumes execution after calling task_exit().
//------------------------------------------------------------------------------
void task_exit(void);

//------------------------------------------------------------------------------
// public
//------------------------------------------------------------------------------
// Initialize the scheduler and task subsystem.
//
// Called by: system initialization (once at boot)
//
// What it does:
//    - Clears current task pointer
//    - Resets the runnable task list to empty
//    - Marks all task slots as UNUSED
//
// Control-flow model:
//    After this call, no tasks exist and the scheduler is idle.
//    Tasks must be created via task_start() before calling sched_start().
//------------------------------------------------------------------------------
void sched_init(void);

//------------------------------------------------------------------------------
// public
//------------------------------------------------------------------------------
// Start task scheduling.
//
// Called by:
//    boot/main after at least one runnable task exists.
//
// What it does:
//    Selects the first runnable task (CLI) and transfers control to it.
//
// Control-flow story:
//    One-way handoff. Does not return.
//------------------------------------------------------------------------------
void sched_start(void);

//------------------------------------------------------------------------------
// public
//------------------------------------------------------------------------------
// Create and register a new runnable task instance.
//
// Called by:
//    boot or running tasks to introduce new work into the system.
//
// What it does:
//    - Allocates an unused slot from the task pool
//    - Initializes the task (name, entry, stack, context)
//    - Links the task into the runnable list
//
// Notes:
//    Each call creates a new live instance of the task entry function.
//    Multiple instances of the same task body may exist simultaneously.
//
// Control-flow story:
//    The task is not executed immediately. It becomes eligible for scheduling
//    and will run when selected by the scheduler.
//------------------------------------------------------------------------------
void task_start(const char* name, void (*entry)(void));
void task_sleep(int);
