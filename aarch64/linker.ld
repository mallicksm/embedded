ENTRY(_start)

SECTIONS
{
   . = 0x40000000;

   .text : {
      *(.text.boot)
      *(.text*)
   }

   .rodata : {
      *(.rodata*)
   }

   .data : {
      *(.data*)
   }

   .bss : {
      *(.bss*)
      *(COMMON)
   }

   . = ALIGN(16);
   . += 0x1000;
   _stack_top = .;
}
