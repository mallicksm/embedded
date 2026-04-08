.section .text.boot
.globl _start
.type _start, @function

_start:
   # ----------------------------------------
   # Set initial stack
   # ----------------------------------------
   la sp, __stack_top

   # ----------------------------------------
   # Zero BSS
   # ----------------------------------------
   la t0, __bss_start
   la t1, __bss_end
1:
   bgeu t0, t1, 2f
   sw zero, 0(t0)
   addi t0, t0, 4
   j 1b
2:
   # ----------------------------------------
   # CSR init (CRITICAL)
   # ----------------------------------------
   csrw mie, zero
   csrw mstatus, zero
   csrw mscratch, zero
   csrw mtvec, zero

   # ----------------------------------------
   # jump to C
   # ----------------------------------------
   call main
3:
   j 3b
