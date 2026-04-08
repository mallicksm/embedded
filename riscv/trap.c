#include "k.h"
#include "proc.h"
#include "trap.h"
#include "printf.h"

#define TIMER_INTERVAL 10000000 // adjust for your platform

void timer_set(uint32_t delta) {
   (void)delta;
   uint64_t now = *MTIME;
   *MTIMECMP = now + delta;
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

   // program timer FIRST
   timer_set(TIMER_INTERVAL);

   // enable timer interrupt
   CSR_SET(mie, MIE_MTIE);

   // enable global interrupts LAST
   CSR_SET(mstatus, MSTATUS_MIE);
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
         timer_set(TIMER_INTERVAL);
         sched_tick();

         if ((g_sched_mode == SCHED_PREE) && g_current_task) {
            task_yield();
         }

         return;
      }

      ASSERT_MSG(0, "trap: unexpected interrupt\n");
   }

   uint32_t mtvec = CSR_READ(mtvec);
   uint32_t mstatus = CSR_READ(mstatus);
   uint32_t mepc = CSR_READ(mepc);

   printf("mstatus=0x%x mepc=0x%x\n", mstatus, mepc);
   printf("mcause=0x%x  mtvec=0x%x\n", cause, mtvec);

   ASSERT_MSG(0, "trap: unexpected exception\n");
}
