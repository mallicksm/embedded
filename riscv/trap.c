#include "k.h"
#include "proc.h"

static const char* exc_name(uint32_t);
static const char* irq_name(uint32_t);
static void trap_dump(void);

//------------------------------------------------------------------------------
// PLIC (stub)
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static inline uint32_t plic_claim(void) {
   return 0; // no PLIC yet
}

static inline void plic_complete(uint32_t irq) {
   (void)irq;
}
//------------------------------------------------------------------------------
// Timer
//------------------------------------------------------------------------------
#define TIMER_INTERVAL 10000000
void timer_set(uint32_t delta) {
   uint64_t now = *MTIME;
   *MTIMECMP = now + delta;
}

//------------------------------------------------------------------------------
// public
//------------------------------------------------------------------------------
extern void trap_entry(void);
void trap_init(void) {
   CSR_WRITE(mtvec, trap_entry);
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

   uint32_t is_irq = cause & MCAUSE_INT;
   uint32_t code   = cause & 0xff;

   //--------------------------------------------------------------------------
   // interrupts
   //--------------------------------------------------------------------------
   if (is_irq) {
      switch (code) {
         case 7: // machine timer interrupt
            timer_set(TIMER_INTERVAL);
            sched_tick();

            if ((g_sched_mode == SCHED_PREE) && g_current_task) {
               task_yield();
            }
            return;

         case 3: // machine software interrupt
            printf("\nINTERRUPT: %s\n", irq_name(code));
            trap_dump();
            PANIC();

         case 11: { // machine external interrupt
            uint32_t irq = plic_claim();

            printf("\nINTERRUPT: %s (irq=%u)\n",
                   irq_name(code), irq);

            if (irq == 0) {
               printf("spurious external interrupt\n");
               trap_dump();
               PANIC();
            }

            // no device support yet
            trap_dump();
            PANIC();

            // future:
            // handle_device(irq);
            plic_complete(irq);
         }

         default:
            printf("\nUNEXPECTED INTERRUPT: %s (%u)\n",
                   irq_name(code), code);
            trap_dump();
            PANIC();
      }
   }

   //--------------------------------------------------------------------------
   // exceptions
   //--------------------------------------------------------------------------
   switch (code) {
      case 11: // ecall (machine)
         CSR_WRITE(mepc, CSR_READ(mepc) + 4);
         return;

      default:
         printf("\nEXCEPTION: %s (%u)\n",
                exc_name(code), code);
         trap_dump();
         PANIC();
   }
}

//------------------------------------------------------------------------------
// private: decode helpers
//------------------------------------------------------------------------------
static const char* exc_name(uint32_t code) {
   switch (code) {
      case 0:  return "instruction address misaligned";
      case 1:  return "instruction access fault";
      case 2:  return "illegal instruction";
      case 3:  return "breakpoint";
      case 4:  return "load address misaligned";
      case 5:  return "load access fault";
      case 6:  return "store/amo address misaligned";
      case 7:  return "store/amo access fault";
      case 8:  return "ecall (user)";
      case 9:  return "ecall (supervisor)";
      case 11: return "ecall (machine)";
      default: return "unknown exception";
   }
}

static const char* irq_name(uint32_t code) {
   switch (code) {
      case 3:  return "machine software interrupt";
      case 7:  return "machine timer interrupt";
      case 11: return "machine external interrupt";
      default: return "unknown interrupt";
   }
}

//------------------------------------------------------------------------------
// private: dump machine state
//------------------------------------------------------------------------------
static void trap_dump(void) {
   uint32_t mepc    = CSR_READ(mepc);
   uint32_t mtval   = CSR_READ(mtval);
   uint32_t mtvec   = CSR_READ(mtvec);
   uint32_t mstatus = CSR_READ(mstatus);

   printf("\n=== TRAP DUMP ===\n");

   // PC & instr where trap occurred
   printf("mepc    = 0x%x  (faulting PC)\n", mepc);
   printf("instr = 0x%x\n", *(uint32_t*)CSR_READ(mepc));

   // extra info (addr or instruction)
   printf("mtval   = 0x%x  (fault addr / bad instr)\n", mtval);

   // trap entry base
   printf("mtvec   = 0x%x  (trap vector base)\n", mtvec);

   // machine state
   printf("mstatus = 0x%x  (MIE=%u MPIE=%u MPP=%u)\n",
          mstatus,
          (mstatus >> 3) & 1,
          (mstatus >> 7) & 1,
          (mstatus >> 11) & 3);

   if (g_current_task) {
      printf("task    = %s\n", g_current_task->name);
   } else {
      printf("task    = <none>\n");
   }
}

