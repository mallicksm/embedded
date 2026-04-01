#pragma once

typedef int (*cli_fn_t)(int argc, char** argv);

struct cmds {
   const char* name;
   cli_fn_t fn;
};

#define REGISTER_CMD(_name, _fn)         \
   static const struct cmds __cmds_##_fn \
      __attribute__((used, section(".cmds"))) = {(_name), (_fn)}

extern const struct cmds __cmds_start[];
extern const struct cmds __cmds_end[];
