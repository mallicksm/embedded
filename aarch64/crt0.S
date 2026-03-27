.section .text.boot
.global _start
.type _start, %function

_start:
   .cfi_startproc
   .cfi_undefined x30
   ldr x0, =__stack_top
   mov sp, x0
   .cfi_def_cfa sp, 0

   mov x29, xzr
   mov x30, xzr

   bl main

1:
   wfi
   b 1b
   .cfi_endproc
