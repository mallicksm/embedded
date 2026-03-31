#include <stdint.h>
#include "uart.h"
#include "cmds.h"
#include "printf.h"
#include "stdlib.h"

static int help(int argc, char** argv) {
   const struct cmds* cmd = __cmds_start;

   (void)argc;
   (void)argv;

   uart_puts("Commands:\n");
   while (cmd < __cmds_end) {
      uart_puts("  ");
      uart_puts(cmd->name);
      uart_puts("\n");
      cmd++;
   }

   return 0;
}

REGISTER("help", help);

static int echo(int argc, char** argv) {
   int i;

   for (i = 1; i < argc; i++) {
      uart_puts(argv[i]);
      if (i + 1 < argc) {
         uart_putc(' ');
      }
   }
   uart_puts("\n");

   return 0;
}

REGISTER("echo", echo);

static int mrd(int argc, char** argv) {
   if (argc < 2) {
      uart_puts("Usage: mrd <addr> [count]\n");
      return -1;
   }

   uint32_t addr = strtoul(argv[1], 0, 0);
   uint32_t count = (argc >= 3) ? strtoul(argv[2], 0, 0) : 1;

   volatile uint32_t* p = (uint32_t*)addr;

   for (uint32_t i = 0; i < count; i++) {
      printf("%p: 0x%08x\n", (void*)&p[i], p[i]);
   }

   return 0;
}

REGISTER("mrd", mrd);

static int mwr(int argc, char** argv) {
   if (argc < 3) {
      uart_puts("Usage: mwr <addr> <data> [count]\n");
      return -1;
   }

   uint32_t addr = strtoul(argv[1], 0, 0);
   uint32_t data = strtoul(argv[2], 0, 0);
   uint32_t count = (argc >= 4) ? strtoul(argv[3], 0, 0) : 1;

   volatile uint32_t* p = (uint32_t*)addr;

   for (uint32_t i = 0; i < count; i++) {
      p[i] = data;
      printf("%p: 0x%08x\n", (void*)&p[i], data);
   }

   return 0;
}

REGISTER("mwr", mwr);
