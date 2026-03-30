#include <stdint.h>
#include "uart.h"
#include "cli_cmd.h"
#include "printf.h"

static int help(int argc, char** argv) {
   const struct cli_cmd* cmd = __cli_cmds_start;

   (void)argc;
   (void)argv;

   uart_puts("Commands:\n");
   while (cmd < __cli_cmds_end) {
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

static unsigned long strtoul(const char* nptr, char** endptr, int base) {
   const char* s = nptr;
   unsigned long result = 0;

   // skip spaces
   while (*s == ' ' || *s == '\t')
      s++;

   // auto-detect base
   if (base == 0) {
      if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
         base = 16;
         s += 2;
      } else {
         base = 10;
      }
   }

   while (*s) {
      int digit;

      if (*s >= '0' && *s <= '9')
         digit = *s - '0';
      else if (*s >= 'a' && *s <= 'f')
         digit = *s - 'a' + 10;
      else if (*s >= 'A' && *s <= 'F')
         digit = *s - 'A' + 10;
      else
         break;

      if (digit >= base)
         break;

      result = result * base + digit;
      s++;
   }

   if (endptr)
      *endptr = (char*)s;

   return result;
}

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
