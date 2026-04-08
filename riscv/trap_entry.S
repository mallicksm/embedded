.section .text
.globl trap_entry
.type trap_entry, @function
.extern g_kstack_top

trap_entry:
   # ----------------------------------------
   # swap: a7 = tf, mscratch = original a7
   # ----------------------------------------
   csrrw a7, mscratch, a7
   beqz a7, 2f # if mscratch == 0 → branch to mret

   # ----------------------------------------
   # Save registers (except a7)
   # ----------------------------------------
   sw ra,  0(a7)

   sw t0,  4(a7)
   sw t1,  8(a7)
   sw t2, 12(a7)
   sw t3, 16(a7)
   sw t4, 20(a7)
   sw t5, 24(a7)
   sw t6, 28(a7)

   sw a0, 32(a7)
   sw a1, 36(a7)
   sw a2, 40(a7)
   sw a3, 44(a7)
   sw a4, 48(a7)
   sw a5, 52(a7)
   sw a6, 56(a7)

   # ----------------------------------------
   # swap back → restore original a7
   # ----------------------------------------
   csrrw a7, mscratch, a7

   # ----------------------------------------
   # t0 = tf pointer (mscratch still holds it)
   # ----------------------------------------
   csrr t0, mscratch

   # ----------------------------------------
   # Save a7 LAST
   # ----------------------------------------
   sw   a7, 60(t0)

   # ----------------------------------------
   # Save mepc LAST (now we have temps free)
   # ----------------------------------------
   csrr t1, mepc
   sw   t1, 64(t0)

   # ----------------------------------------
   # set kernel stack
   # ----------------------------------------
   la t0, g_kstack_top   # sp = top of stack
   lw sp, 0(t0)
   # ----------------------------------------
   # Call handler
   # ----------------------------------------
   call trap_handler

   # ----------------------------------------
   # Load tf pointer
   # ----------------------------------------
   csrr a7, mscratch

   # restore mepc
   lw t0, 64(a7)
   csrw mepc, t0

   # ----------------------------------------
   # Restore registers
   # ----------------------------------------
   lw ra,  0(a7)

   lw t0,  4(a7)
   lw t1,  8(a7)
   lw t2, 12(a7)
   lw t3, 16(a7)
   lw t4, 20(a7)
   lw t5, 24(a7)
   lw t6, 28(a7)

   lw a0, 32(a7)
   lw a1, 36(a7)
   lw a2, 40(a7)
   lw a3, 44(a7)
   lw a4, 48(a7)
   lw a5, 52(a7)
   lw a6, 56(a7)
   lw a7, 60(a7)

   1:
   mret
   2:
   csrrw a7, mscratch, a7
   mret
.size trap_entry, .-trap_entry
