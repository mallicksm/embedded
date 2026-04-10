#include "k.h"
#include "proc.h"

#define TIMER_INTERVAL 10000000 // adjust for your platform

void timer_set(uint32_t delta) {
   (void)delta;
   uint64_t now = *MTIME;
   *MTIMECMP = now + delta;
}

//------------------------------------------------------------------------------
// public
//------------------------------------------------------------------------------
// Initialize trap system WITHOUT enabling interrupts.
//
// Called during boot.
//
// What it does:
//    - installs trap vector
//    - enables timer interrupt source (mie)
//    - programs timer
//
// Invariant:
//    global interrupts remain OFF after this call
//------------------------------------------------------------------------------
extern void trap_entry(void);
void trap_init(void) {
   CSR_WRITE(mtvec, trap_entry);

   // enable timer interrupt source (but still globally masked)
   CSR_SET(mie, MIE_MTIE);

   timer_set(TIMER_INTERVAL);

   ASSERT_INTR_OFF();
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
