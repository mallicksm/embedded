// startup.s
.section .text.boot
.global _start
.type _start, %function

_start:
   .cfi_startproc
   .cfi_undefined x30
   // set up stack pointer
   ldr x0, =_stack_top
   mov sp, x0
   .cfi_def_cfa sp, 0

   // optionally clear frame pointer / link register
   mov x29, xzr
   mov x30, xzr

   // call main()
   bl main

1:
   wfi
   b 1b
   .cfi_endproc
