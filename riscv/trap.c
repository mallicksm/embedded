#include "k.h"
#include "proc.h"

#define TIMER_INTERVAL 100000   // adjust for your platform

void timer_set(void) {
   uint64_t now = *MTIME;
   *MTIMECMP = now + TIMER_INTERVAL;
}
//------------------------------------------------------------------------------
// public
//------------------------------------------------------------------------------
// Enable traps and timer interrupt.
//
// Called once during boot before sched_start().
//
// What it does:
//    - Sets trap vector (mtvec)
//    - Enables machine timer interrupt (mie.MTIE)
//    - Enables global interrupts (mstatus.MIE)
//
// System invariant:
//    trap_handler() must be installed before enabling interrupts.
//------------------------------------------------------------------------------
extern void trap_entry(void);

void trap_enable(void) {
   // install trap vector
   CSR_WRITE(mtvec, trap_entry);

   // enable timer interrupt
   CSR_SET(mie, MIE_MTIE);

   // enable global interrupts
   CSR_SET(mstatus, MSTATUS_MIE);

   timer_set();
}

//------------------------------------------------------------------------------
// public (called from trap_entry.S)
//------------------------------------------------------------------------------
// Trap handler.
//
// Called on:
//    - timer interrupt
//    - exceptions (unexpected → panic)
//
// What it does:
//    - Decodes mcause
//    - Handles timer interrupt:
//         * advances scheduler time
//         * re-arms timer
//
// Control-flow:
//    Returns back to interrupted context via mret (in trap_entry.S).
//------------------------------------------------------------------------------
void trap_handler(void) {
   uint32_t cause = CSR_READ(mcause);

   if (cause & MCAUSE_INT) {
      uint32_t irq = cause & 0xff;

      if (irq == 7) {
         timer_set();
         sched_tick();
         return;
      }

      ASSERT_MSG(0, "trap: unexpected interrupt\n");
   }

   ASSERT_MSG(0, "trap: unexpected exception\n");
}
