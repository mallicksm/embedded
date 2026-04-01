#pragma once

struct tasks {
   const char* name;
   void (*fn)(void);
};

#define REGISTER_TASK(_name, _fn)          \
   static const struct tasks __tasks_##_fn \
      __attribute__((used, section(".tasks"))) = {(_name), (_fn)}

extern const struct tasks __tasks_start[];
extern const struct tasks __tasks_end[];
