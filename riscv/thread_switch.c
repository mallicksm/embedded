#include "k.h"
#include "proc.h"

__attribute__((naked)) void thread_switch(struct thread_context *old,
                                          struct thread_context *new)
{
   // if (old == NULL) skip save
   ASM("beqz a0, 1f");

   // ---- SAVE (callee-saved) ----
   SAVE(a0, s0, struct thread_context);
   SAVE(a0, s1, struct thread_context);
   SAVE(a0, s2, struct thread_context);
   SAVE(a0, s3, struct thread_context);
   SAVE(a0, s4, struct thread_context);
   SAVE(a0, s5, struct thread_context);
   SAVE(a0, s6, struct thread_context);
   SAVE(a0, s7, struct thread_context);
   SAVE(a0, s8, struct thread_context);
   SAVE(a0, s9, struct thread_context);
   SAVE(a0, s10, struct thread_context);
   SAVE(a0, s11, struct thread_context);

   SAVE(a0, ra, struct thread_context);
   SAVE(a0, sp, struct thread_context);

   ASM("1:");

   // ---- RESTORE ----
   RESTORE(a1, s0, struct thread_context);
   RESTORE(a1, s1, struct thread_context);
   RESTORE(a1, s2, struct thread_context);
   RESTORE(a1, s3, struct thread_context);
   RESTORE(a1, s4, struct thread_context);
   RESTORE(a1, s5, struct thread_context);
   RESTORE(a1, s6, struct thread_context);
   RESTORE(a1, s7, struct thread_context);
   RESTORE(a1, s8, struct thread_context);
   RESTORE(a1, s9, struct thread_context);
   RESTORE(a1, s10, struct thread_context);
   RESTORE(a1, s11, struct thread_context);

   RESTORE(a1, ra, struct thread_context);
   RESTORE(a1, sp, struct thread_context);

   ASM("ret");
}
