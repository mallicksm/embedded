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
//    TASK_UNUSED
//       Task slot is free and not in use.
//
//    TASK_RUNNABLE
//       Task is eligible to be chosen by the scheduler.
//
//    TASK_RUNNING
//       Task is the one currently executing.
//
//    TASK_BLOCKED
//       Task exists but cannot run until some event wakes it.
//
//    TASK_DONE
//       Task has finished and should not be scheduled again.
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
//    name
//       Human-readable name for debug / diagnostics.
//
//    ctx
//       Saved machine context used by thread_switch().
//
//    state
//       Scheduler-visible run state.
//
//    entry
//       Task entry function. This is what the first-entry trampoline invokes
//       the first time the task is scheduled.
//
//    stack_base / stack_size
//       Memory owned by this task for its stack.
//
//    next
//       Link field for task list / runnable queue management.
//
// Control-flow story:
//    A task is the scheduler-owned object. Its ctx field is only one part of
//    it. The scheduler reasons about the whole task; thread_switch() only
//    reasons about task->ctx.
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
// Low-level context switch primitive.
//
// Called by:
//    scheduler/task-switching code, not by ordinary task code.
//
// What it does:
//    Saves the currently running task's callee-saved register set into *old,
//    restores the next task's saved register set from *new, then resumes
//    execution in that restored context.
//
// Control-flow story:
//    thread_switch() does not return in the usual sense. Its final ret
//    continues execution using the restored context from *new.
//
//    - If *new belongs to a task that has run before, execution resumes
//      where that task last yielded.
//    - If *new belongs to a never-before-run task, execution begins at the
//      first-entry trampoline installed by task_init().
//
// Important:
//    - switches thread contexts, not full trap state
//    - implemented in assembly
//------------------------------------------------------------------------------
void thread_switch(struct thread_context* old,
                   struct thread_context* new);

//------------------------------------------------------------------------------
// Initialize a never-before-run task.
//
// Called by:
//    boot/init code for the first task, and later by running tasks that want
//    to create/start additional tasks.
//
// What it does:
//    Fills in the task control block and builds the task's initial saved
//    thread context.
//
//    In particular, it:
//       - records the task's identity and entry function
//       - records the task's stack ownership
//       - marks the task runnable
//       - sets the saved SP to the top of the task's stack
//       - sets the saved RA to the first-entry trampoline
//
// Control-flow story:
//    task_init() does not run the task immediately. It prepares the task so
//    that when the scheduler later chooses it and thread_switch() restores its
//    context, the final ret enters the trampoline, which then starts the
//    task's entry function on its own stack.
//------------------------------------------------------------------------------
void task_init(struct task* task,
               const char* name,
               void (*entry)(void),
               uint8_t* stack_base,
               uint32_t stack_size);

//------------------------------------------------------------------------------
// Yield the CPU voluntarily from the current task.
//
// Called by:
//    the currently running task.
//
// What it does:
//    Gives control back to the scheduler so another runnable task may be
//    chosen.
//
// Control-flow story:
//    From the task's point of view, execution later continues immediately
//    after task_yield() when this task is scheduled again.
//
//    That is why task_yield() often reads like a nop in task code, even
//    though it may allow many other tasks to run before returning here.
//------------------------------------------------------------------------------
void task_yield(void);

//------------------------------------------------------------------------------
// Terminate the current task.
//
// Called by:
//    the current task when it is finished, or by task startup/exit machinery
//    if a task entry function returns unexpectedly.
//
// What it does:
//    Marks the current task as done so the scheduler will not run it again.
//
// Control-flow story:
//    task_exit() is not expected to return to the exiting task. Control is
//    handed back to the scheduler, which should choose some other runnable
//    task.
//------------------------------------------------------------------------------
void task_exit(void);

//------------------------------------------------------------------------------
// Return the currently running task.
//
// Called by:
//    scheduler code, task code, and internal task startup/trampoline code.
//
// What it does:
//    Returns the scheduler's current-task pointer.
//
// Control-flow story:
//    Pure query helper; does not change scheduling state.
//------------------------------------------------------------------------------
struct task* task_current(void);

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
// Pick the next runnable task.
//
// Called by:
//    scheduler code when it needs to decide which task should run next.
//
// What it does:
//    Applies scheduler policy to choose one runnable task.
//
// Control-flow story:
//    This function only chooses; it does not itself switch contexts.
//    The actual handoff happens later through thread_switch().
//------------------------------------------------------------------------------
struct task* sched_pick(void);
