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
// public
//------------------------------------------------------------------------------
void task_start(const char* name, void (*entry)(void));
void task_yield(void);
void task_exit(void);
void sched_init(void);
void sched_start(void);
void task_sleep(int);
void sched_tick(void);
