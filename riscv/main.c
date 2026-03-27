#include <stdint.h>
#include "uart.h"
#include "cli.h"

#define CMD_BUF_SIZE    128

static void uart_prompt(void) {
   uart_puts("> ");
}

#define MAX_ARGS 16

static int streq(const char *a, const char *b) {
   while (*a && *b) {
      if (*a != *b) {
         return 0;
      }
      a++;
      b++;
   }

   return (*a == '\0' && *b == '\0');
}

static int cli_tokenize(char *buf, char **argv, int max_args) {
   int argc = 0;
   char *p = buf;

   while (*p && argc < max_args) {
      while (*p == ' ' || *p == '\t') {
         p++;
      }

      if (*p == '\0') {
         break;
      }

      argv[argc++] = p;

      while (*p && *p != ' ' && *p != '\t') {
         p++;
      }

      if (*p == '\0') {
         break;
      }

      *p = '\0';
      p++;
   }

   return argc;
}

static const struct cli_cmd *cli_find(const char *name) {
   const struct cli_cmd *cmd = __cli_cmds_start;

   while (cmd < __cli_cmds_end) {
      if (streq(cmd->name, name)) {
         return cmd;
      }
      cmd++;
   }

   return 0;
}

static void cli_exec(char *buf) {
   char *argv[MAX_ARGS];
   int argc;
   const struct cli_cmd *cmd;

   argc = cli_tokenize(buf, argv, MAX_ARGS);
   if (argc == 0) {
      return;
   }

   cmd = cli_find(argv[0]);
   if (cmd == 0) {
      uart_puts("Unknown command: ");
      uart_puts(argv[0]);
      uart_puts("\n");
      return;
   }

   cmd->fn(argc, argv);
}

int main(void) {
   char buf[CMD_BUF_SIZE];
   int idx = 0;

   uart_puts("Hello from RV32 bare metal clang on QEMU virt\n");
   uart_puts("Type a command and press Enter.\n");
   uart_prompt();

   for (;;) {
      char c = uart_getc();

      if (c == '\r' || c == '\n') {
         uart_puts("\n");
         buf[idx] = '\0';

         cli_exec(buf);

         idx = 0;
         uart_prompt();
      } else if (c == 0x7f || c == 0x08) {
         if (idx > 0) {
            idx--;
            uart_putc('\b');
            uart_putc(' ');
            uart_putc('\b');
         }
      } else {
         if (idx < (CMD_BUF_SIZE - 1)) {
            buf[idx++] = c;
            uart_putc(c);
         }
      }
   }

   return 0;
}
