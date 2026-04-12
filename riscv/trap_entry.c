#include "k.h"
#include "proc.h"

//
// Global trapframe (single CPU model)
//
struct trapframe g_tf;
extern void trap_handler(void);

__attribute__((naked)) void trap_entry(void) {
   // swap a0 <-> mscratch (get trapframe pointer)
   ASM("csrrw a0, mscratch, a0");

   // ---- SAVE_A0 ----
   SAVE(a0, ra, struct trapframe);
   SAVE(a0, sp, struct trapframe);
   SAVE(a0, gp, struct trapframe);
   SAVE(a0, tp, struct trapframe);

   SAVE(a0, t0, struct trapframe);
   SAVE(a0, t1, struct trapframe);
   SAVE(a0, t2, struct trapframe);

   SAVE(a0, s0, struct trapframe);
   SAVE(a0, s1, struct trapframe);

   SAVE(a0, a1, struct trapframe);
   SAVE(a0, a2, struct trapframe);
   SAVE(a0, a3, struct trapframe);
   SAVE(a0, a4, struct trapframe);
   SAVE(a0, a5, struct trapframe);
   SAVE(a0, a6, struct trapframe);
   SAVE(a0, a7, struct trapframe);

   SAVE(a0, s2, struct trapframe);
   SAVE(a0, s3, struct trapframe);
   SAVE(a0, s4, struct trapframe);
   SAVE(a0, s5, struct trapframe);
   SAVE(a0, s6, struct trapframe);
   SAVE(a0, s7, struct trapframe);
   SAVE(a0, s8, struct trapframe);
   SAVE(a0, s9, struct trapframe);
   SAVE(a0, s10, struct trapframe);
   SAVE(a0, s11, struct trapframe);

   SAVE(a0, t3, struct trapframe);
   SAVE(a0, t4, struct trapframe);
   SAVE(a0, t5, struct trapframe);
   SAVE(a0, t6, struct trapframe);

   // save original a0
   ASM("csrr t0, mscratch");
   SAVE(a0, t0, struct trapframe);

   asm volatile(
      "la sp, g_kstack\n"
      "addi sp, sp, %0\n"
      :
      : "i"(KSTACK_SIZE)
      : "sp", "memory");

   ASM("call trap_handler");
   ASM("csrr a0, mscratch");

   // ---- RESTORE_A0 ----
   RESTORE(a0, ra, struct trapframe);
   RESTORE(a0, sp, struct trapframe);
   RESTORE(a0, gp, struct trapframe);
   RESTORE(a0, tp, struct trapframe);

   RESTORE(a0, t0, struct trapframe);
   RESTORE(a0, t1, struct trapframe);
   RESTORE(a0, t2, struct trapframe);

   RESTORE(a0, s0, struct trapframe);
   RESTORE(a0, s1, struct trapframe);

   RESTORE(a0, a1, struct trapframe);
   RESTORE(a0, a2, struct trapframe);
   RESTORE(a0, a3, struct trapframe);
   RESTORE(a0, a4, struct trapframe);
   RESTORE(a0, a5, struct trapframe);
   RESTORE(a0, a6, struct trapframe);
   RESTORE(a0, a7, struct trapframe);

   RESTORE(a0, s2, struct trapframe);
   RESTORE(a0, s3, struct trapframe);
   RESTORE(a0, s4, struct trapframe);
   RESTORE(a0, s5, struct trapframe);
   RESTORE(a0, s6, struct trapframe);
   RESTORE(a0, s7, struct trapframe);
   RESTORE(a0, s8, struct trapframe);
   RESTORE(a0, s9, struct trapframe);
   RESTORE(a0, s10, struct trapframe);
   RESTORE(a0, s11, struct trapframe);

   RESTORE(a0, t3, struct trapframe);
   RESTORE(a0, t4, struct trapframe);
   RESTORE(a0, t5, struct trapframe);
   RESTORE(a0, t6, struct trapframe);

   RESTORE(a0, a0, struct trapframe);

   // return from trap
   ASM("mret");
}
