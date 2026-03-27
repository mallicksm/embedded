#pragma once

typedef int (*cli_fn_t)(int argc, char** argv);

struct cli_cmd {
   const char* name;
   cli_fn_t fn;
};

#define REGISTER(_name, _fn)                   \
   static const struct cli_cmd __cli_cmd_##_fn \
      __attribute__((used, section(".cli_cmds"))) = {(_name), (_fn)}

extern const struct cli_cmd __cli_cmds_start[];
extern const struct cli_cmd __cli_cmds_end[];
