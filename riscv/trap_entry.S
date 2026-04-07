.section .text
.globl trap_entry
.type trap_entry, @function

trap_entry:
   # --------------------------------------------------
   # Bootstrap (your pattern)
   # --------------------------------------------------
   csrrw t6, mscratch, t6     # t6 = kstack, mscratch = user t6
   mv    sp, t6               # switch to kernel stack
   csrrw t6, mscratch, t6     # t6 = user t6, mscratch = kstack

   # --------------------------------------------------
   # Allocate trapframe (C layout)
   # --------------------------------------------------
   addi  sp, sp, -64

   # --------------------------------------------------
   # Save caller-saved (uniform, no special cases)
   # --------------------------------------------------
   sw ra,  0(sp)

   sw t0,  4(sp)
   sw t1,  8(sp)
   sw t2, 12(sp)
   sw t3, 16(sp)
   sw t4, 20(sp)
   sw t5, 24(sp)
   sw t6, 28(sp)

   sw a0, 32(sp)
   sw a1, 36(sp)
   sw a2, 40(sp)
   sw a3, 44(sp)
   sw a4, 48(sp)
   sw a5, 52(sp)
   sw a6, 56(sp)
   sw a7, 60(sp)

   # --------------------------------------------------
   # Call handler
   # --------------------------------------------------
   call  trap_handler

   # --------------------------------------------------
   # Restore caller-saved
   # --------------------------------------------------
   lw ra,  0(sp)

   lw t0,  4(sp)
   lw t1,  8(sp)
   lw t2, 12(sp)
   lw t3, 16(sp)
   lw t4, 20(sp)
   lw t5, 24(sp)
   lw t6, 28(sp)

   lw a0, 32(sp)
   lw a1, 36(sp)
   lw a2, 40(sp)
   lw a3, 44(sp)
   lw a4, 48(sp)
   lw a5, 52(sp)
   lw a6, 56(sp)
   lw a7, 60(sp)

   addi sp, sp, 64

   mret

.size trap_entry, .-trap_entry
