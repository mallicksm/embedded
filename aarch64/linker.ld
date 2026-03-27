ENTRY(_start)

MEMORY
{
   RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 128M
}

__uart0_base = 0x09000000;

SECTIONS
{
   . = ORIGIN(RAM);

   .text : ALIGN(4)
   {
      KEEP(*(.text.boot))
      *(.text*)
   } > RAM

   .rodata : ALIGN(4)
   {
      *(.rodata*)
   } > RAM

   .data : ALIGN(4)
   {
      __data_start = .;
      *(.data*)
      __data_end = .;
   } > RAM

   .bss : ALIGN(4)
   {
      __bss_start = .;
      *(.bss*)
      *(COMMON)
      __bss_end = .;
   } > RAM

   . = ALIGN(16);
   __stack_top = ORIGIN(RAM) + LENGTH(RAM);
}
