#include <stdint.h>
#include "uart.h"
#include "cmds.h"
#include "proc.h"
#include "printf.h"
#include "string.h"

static void prompt(void) {
   printf("> ");
}

static int cli_tokenize(char* buf, char** argv, int max_args) {
   int argc = 0;
   char* p = buf;

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

static const struct cmds* cli_find(const char* name) {
   const struct cmds* cmd = __cmds_start;

   while (cmd < __cmds_end) {
      if (strcmp(cmd->name, name) == 0) {
         return cmd;
      }
      cmd++;
   }

   return 0;
}

#define MAX_ARGS 16
static void cli_exec(char* buf) {
   char* argv[MAX_ARGS];
   int argc;
   const struct cmds* cmd;

   argc = cli_tokenize(buf, argv, MAX_ARGS);
   if (argc == 0) {
      return;
   }

   cmd = cli_find(argv[0]);
   if (cmd == 0) {
      printf("Unknown command: %s\n", argv[0]);
      return;
   }

   cmd->fn(argc, argv);
}

#define CMD_BUF_SIZE 128
void cli(void) {
   char buf[CMD_BUF_SIZE];
   int idx = 0;

   printf("Hello from RV32 bare metal clang on QEMU virt\n");
   printf("Type a command and press Enter.\n");
   prompt();

   for (;;) {
      char c = uart_getc();

      if (c == '\r' || c == '\n') {
         printf("\n");
         buf[idx] = '\0';

         cli_exec(buf);

         idx = 0;
         prompt();

         task_yield();
      } else if (c == 0x7f || c == 0x08) {
         if (idx > 0) {
            idx--;
            printf("\b \b");
         }
      } else {
         if (idx < (CMD_BUF_SIZE - 1)) {
            buf[idx++] = c;
            printf("%c", c);
         }
      }
   }
}

static struct task cli_task;
static uint8_t cli_stack[1024];

void task_cli (void) {
   task_init(&cli_task, "cli", cli, cli_stack, sizeof(cli_stack));
}
