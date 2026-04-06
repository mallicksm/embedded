.section .text
.globl trap_entry
.type trap_entry, @function

trap_entry:
   addi sp, sp, -96

   sw ra,  0(sp)

   sw t0,  4(sp)
   sw t1,  8(sp)
   sw t2, 12(sp)
   sw a0, 16(sp)
   sw a1, 20(sp)
   sw a2, 24(sp)
   sw a3, 28(sp)
   sw a4, 32(sp)
   sw a5, 36(sp)
   sw a6, 40(sp)
   sw a7, 44(sp)

   sw s0, 48(sp)
   sw s1, 52(sp)
   sw s2, 56(sp)
   sw s3, 60(sp)
   sw s4, 64(sp)
   sw s5, 68(sp)
   sw s6, 72(sp)
   sw s7, 76(sp)
   sw s8, 80(sp)
   sw s9, 84(sp)
   sw s10, 88(sp)
   sw s11, 92(sp)

   call trap_handler

   lw ra,  0(sp)
   lw t0,  4(sp)
   lw t1,  8(sp)
   lw t2, 12(sp)
   lw a0, 16(sp)
   lw a1, 20(sp)
   lw a2, 24(sp)
   lw a3, 28(sp)
   lw a4, 32(sp)
   lw a5, 36(sp)
   lw a6, 40(sp)
   lw a7, 44(sp)

   lw s0, 48(sp)
   lw s1, 52(sp)
   lw s2, 56(sp)
   lw s3, 60(sp)
   lw s4, 64(sp)
   lw s5, 68(sp)
   lw s6, 72(sp)
   lw s7, 76(sp)
   lw s8, 80(sp)
   lw s9, 84(sp)
   lw s10, 88(sp)
   lw s11, 92(sp)

   addi sp, sp, 96

   mret

.size trap_entry, .-trap_entry
