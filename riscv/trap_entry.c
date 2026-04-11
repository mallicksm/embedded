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
   SAVE_A0(ra, struct trapframe);
   SAVE_A0(sp, struct trapframe);
   SAVE_A0(gp, struct trapframe);
   SAVE_A0(tp, struct trapframe);

   SAVE_A0(t0, struct trapframe);
   SAVE_A0(t1, struct trapframe);
   SAVE_A0(t2, struct trapframe);

   SAVE_A0(s0, struct trapframe);
   SAVE_A0(s1, struct trapframe);

   SAVE_A0(a1, struct trapframe);
   SAVE_A0(a2, struct trapframe);
   SAVE_A0(a3, struct trapframe);
   SAVE_A0(a4, struct trapframe);
   SAVE_A0(a5, struct trapframe);
   SAVE_A0(a6, struct trapframe);
   SAVE_A0(a7, struct trapframe);

   SAVE_A0(s2, struct trapframe);
   SAVE_A0(s3, struct trapframe);
   SAVE_A0(s4, struct trapframe);
   SAVE_A0(s5, struct trapframe);
   SAVE_A0(s6, struct trapframe);
   SAVE_A0(s7, struct trapframe);
   SAVE_A0(s8, struct trapframe);
   SAVE_A0(s9, struct trapframe);
   SAVE_A0(s10, struct trapframe);
   SAVE_A0(s11, struct trapframe);

   SAVE_A0(t3, struct trapframe);
   SAVE_A0(t4, struct trapframe);
   SAVE_A0(t5, struct trapframe);
   SAVE_A0(t6, struct trapframe);

   // save original a0
   ASM("csrr t0, mscratch");
   SAVE_A0(a0, struct trapframe);

   ASM("call trap_handler");

   // ---- RESTORE_A0 ----
   RESTORE_A0(ra, struct trapframe);
   RESTORE_A0(sp, struct trapframe);
   RESTORE_A0(gp, struct trapframe);
   RESTORE_A0(tp, struct trapframe);

   RESTORE_A0(t0, struct trapframe);
   RESTORE_A0(t1, struct trapframe);
   RESTORE_A0(t2, struct trapframe);

   RESTORE_A0(s0, struct trapframe);
   RESTORE_A0(s1, struct trapframe);

   RESTORE_A0(a1, struct trapframe);
   RESTORE_A0(a2, struct trapframe);
   RESTORE_A0(a3, struct trapframe);
   RESTORE_A0(a4, struct trapframe);
   RESTORE_A0(a5, struct trapframe);
   RESTORE_A0(a6, struct trapframe);
   RESTORE_A0(a7, struct trapframe);

   RESTORE_A0(s2, struct trapframe);
   RESTORE_A0(s3, struct trapframe);
   RESTORE_A0(s4, struct trapframe);
   RESTORE_A0(s5, struct trapframe);
   RESTORE_A0(s6, struct trapframe);
   RESTORE_A0(s7, struct trapframe);
   RESTORE_A0(s8, struct trapframe);
   RESTORE_A0(s9, struct trapframe);
   RESTORE_A0(s10, struct trapframe);
   RESTORE_A0(s11, struct trapframe);

   RESTORE_A0(t3, struct trapframe);
   RESTORE_A0(t4, struct trapframe);
   RESTORE_A0(t5, struct trapframe);
   RESTORE_A0(t6, struct trapframe);

   RESTORE_A0(a0, struct trapframe);

   // return from trap
   ASM("mret");
}
