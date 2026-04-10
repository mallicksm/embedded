//------------------------------------------------------------------------------
// System model:
//
//   - CLI is the primary OS task and is started at boot.
//   - User tasks are registered via REGISTER_PROG() and started via CLI.
//   - At least one task (CLI) must always remain runnable.
//------------------------------------------------------------------------------
#include "k.h"
#include <stdint.h>
#include "cmds.h"
#include "progs.h"
#include "proc.h"

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

static const struct cmds* cmd_find(const char* name) {
   const struct cmds* cmd = __cmds_start;

   while (cmd < __cmds_end) {
      if (strcmp(cmd->name, name) == 0) {
         return cmd;
      }
      cmd++;
   }
   return 0;
}
static const struct progs* prog_find(const char* name) {
   const struct progs* prog;

   prog = __progs_start;
   while (prog < __progs_end) {
      if (strcmp(prog->name, name) == 0) {
         return prog;
      }
      prog++;
   }
   return 0;
}

#define MAX_ARGS 16
static void cli_exec(char* buf) {
   char* argv[MAX_ARGS];
   int argc;

   argc = cli_tokenize(buf, argv, MAX_ARGS);
   if (argc == 0) {
      return;
   }

   const struct cmds* cmd = cmd_find(argv[0]);
   if (cmd) {
      cmd->fn(argc, argv);
      return;
   }

   const struct progs* prog = prog_find(argv[0]);
   if (prog) {
      task_spawn(prog->name, prog->fn);
      return;
   }

   printf("Unknown command: %s\n", argv[0]);
}

//------------------------------------------------------------------------------
// System invariant:
//    The CLI task is started at boot and must not exit.
//    It guarantees that at least one runnable task always exists.
//------------------------------------------------------------------------------
#define CMD_BUF_SIZE 128
void prog_cli(void) {
   char buf[CMD_BUF_SIZE];
   int idx = 0;

   printf("Hello from AITMOS, an RV32 bare metal OS using clang on QEMU virt\n");
   printf("Type a command and press Enter.\n");
   prompt();

   for (;;) {
      int c = uart_getc_nonblock();
      if (c < 0) {
         task_yield();
         continue;
      }

      if (c == '\n')
         continue;

      if (c == '\r') {
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
            buf[idx++] = (char)c;
            printf("%c", c);
         }
      }
   }
}
