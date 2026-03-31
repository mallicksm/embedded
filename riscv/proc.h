#pragma once
#include <stdint.h>

//------------------------------------------------------------------------------
// Saved machine context for a suspended task.
//
// This is the low-level register set handled by thread_switch().
// It is not a full trap frame and it is not "all CPU state".
//
// It holds the callee-preserved register state that must survive
// across a task switch, plus the stack pointer and return address
// needed to resume execution later.
//
// For RV32, this means:
//    ra       return address
//    sp       stack pointer
//    s0-s11   callee-saved registers
//------------------------------------------------------------------------------
struct thread_context {
   uint32_t ra;
   uint32_t sp;

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
};

//------------------------------------------------------------------------------
// Scheduler-visible state of a task.
//
// TASK_UNUSED
//    Slot is free / not in use.
//
// TASK_RUNNABLE
//    Task is ready to run and may be chosen by the scheduler.
//
// TASK_RUNNING
//    Task is the one currently executing.
//
// TASK_BLOCKED
//    Task cannot run until some event wakes it.
//
// TASK_DONE
//    Task has exited and will not run again.
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
// This is the scheduler's bookkeeping object for one task.
// It contains:
//
//    name
//       Human-readable task name for debug / diagnostics.
//
//    ctx
//       Saved execution context used by thread_switch().
//
//    state
//       Current scheduler state.
//
//    entry
//       Task entry function used when the task starts for the first time.
//
//    stack_base / stack_size
//       Memory region owned by this task for its stack.
//
//    next
//       Link field for runnable list / task queue management.
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
// Save the current task's callee-preserved execution state into *old,
// restore the next task's saved execution state from *new, and resume
// execution in the new task.
//
// This function is implemented in assembly.
//
// Important:
//    - it switches thread contexts, not full trap state
//    - it returns in the restored task's context
//    - it is the fundamental mechanism underneath task_yield()
//      and scheduler handoff
//------------------------------------------------------------------------------
void thread_switch(struct thread_context* old,
                   struct thread_context* new);

//------------------------------------------------------------------------------
// Initialize one task object.
//
// Prepares the task control block so the scheduler can later run it.
// This sets up identity, entry point, stack ownership, and initial state.
//
// This does not itself run the task.
//------------------------------------------------------------------------------
void task_init(struct task* task,
               const char* name,
               void (*entry)(void),
               uint8_t* stack_base,
               uint32_t stack_size);

//------------------------------------------------------------------------------
// Yield the CPU voluntarily from the current task.
//
// The current task gives control back to the scheduler so another
// runnable task may be chosen. If the scheduler picks the same task
// again, execution resumes immediately after this call appearing like a nop().
//------------------------------------------------------------------------------
void task_yield(void);

//------------------------------------------------------------------------------
// Terminate the current task.
//
// Marks the current task as finished and transfers control back to the
// scheduler. A task in TASK_DONE state is not expected to run again.
//------------------------------------------------------------------------------
void task_exit(void);

//------------------------------------------------------------------------------
// Return the task that is currently running.
//
// This is the scheduler's current task pointer exposed as a helper.
//------------------------------------------------------------------------------
struct task* task_current(void);

//------------------------------------------------------------------------------
// Initialize scheduler global state.
//
// Sets up internal scheduler bookkeeping before any tasks are started.
//------------------------------------------------------------------------------
void sched_init(void);

//------------------------------------------------------------------------------
// Start task scheduling.
//
// Transfer control from boot/setup code into the scheduler so it may
// pick and run the first runnable task.
//------------------------------------------------------------------------------
void sched_start(void);

//------------------------------------------------------------------------------
// Pick the next runnable task.
//
// Scheduler policy function that selects which task should run next.
// The exact policy may be round-robin or something else later.
//------------------------------------------------------------------------------
struct task* sched_pick(void);
