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
//    name:       Human-readable name for debug / diagnostics.
//    ctx:        Saved machine context used by thread_switch().
//    state:      Scheduler-visible run state.
//    entry:      Task entry function. This is what the first-entry trampoline invokes the first time the task is scheduled.
//    stack_base: Stack Base Memory owned by this task.
//    stack_size: Stack Memory Size owned by this task.
//    next:       Link field for task list / runnable queue management.
//
// Control-flow story:
//    A task is a scheduler-owned object.
//    thread_switch() uses task->ctx.
//------------------------------------------------------------------------------
struct task {
   const char* name;
   struct thread_context ctx;
   enum task_state state;
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
// Terminate the current task.
//
// Called by: task_trampoline()
//    the current task when it is finished, the task startup/exit machinery
//
// What it does:
//    - removes the task from the queue
//    - 
//    Marks the current task as done so the scheduler will not run it again.
//
// Control-flow story:
//    task_exit() is not expected to return to the exiting task. Control is
//    handed back to the scheduler, which should choose some other runnable
//    task.
//------------------------------------------------------------------------------
void task_exit(void);

//------------------------------------------------------------------------------
// Initialize scheduler global state.
//
// Called by:
//    boot/main before any tasks are started.
//
// What it does:
//    Resets scheduler bookkeeping and prepares the scheduler to accept tasks.
//
// Control-flow story:
//    Does not start task execution by itself. It only prepares scheduler state.
//------------------------------------------------------------------------------
void sched_init(void);

//------------------------------------------------------------------------------
// Start task scheduling.
//
// Called by:
//    boot/main after at least one runnable task has been initialized.
//
// What it does:
//    Chooses the first runnable task and transfers control from boot/setup
//    code into scheduled task execution.
//
// Control-flow story:
//    In the normal model, sched_start() is a one-way handoff from boot code
//    into the task world. The first thread_switch() performed here begins or
//    resumes task execution, and boot code is not expected to continue running
//    as the primary control loop.
//------------------------------------------------------------------------------
void sched_start(void);

//------------------------------------------------------------------------------
// Start one new live instance from a registered task template.
//
// The input task supplies the template information:
//    - task name
//    - task entry function
//    - desired stack size
//
// task_start() does not make that registered task object itself live.
// Instead, it allocates an unused slot from the internal task pool,
// initializes that slot from the template, links the new live task into
// the runnable task list, and leaves the registered template object inert.
//
// This allows multiple live instances of the same registered task body.
//------------------------------------------------------------------------------
void task_start(const char* name, void (*entry)(void));
