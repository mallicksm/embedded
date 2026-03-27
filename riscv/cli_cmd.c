#include <stdint.h>
#include "uart.h"
#include "cli.h"

static int help(int argc, char **argv) {
   const struct cli_cmd *cmd = __cli_cmds_start;

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

static int echo(int argc, char **argv) {
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
