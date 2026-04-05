#pragma once

struct tasks {
   const char* name;
   void (*fn)(void);
};

#define REGISTER_PROG(_name, _fn)          \
   static const struct tasks __tasks_##_fn \
      __attribute__((used, section(".tasks"))) = {(_name), (_fn)}

extern const struct tasks __tasks_start[];
extern const struct tasks __tasks_end[];

#define TASK_COOP 0
#define TASK_PREE 1
#define DEFINE_PROG(name, type)                         \
   void prog_##name(void) {                             \
      for (;;) {                                        \
         printf("Task (%s):" #name "\n",                \
                (type) == TASK_COOP ? "coop" : "pree"); \
         if ((type) == TASK_COOP) {                     \
            task_yield();                               \
         }                                              \
      }                                                 \
   }                                                    \
   REGISTER_PROG(#name, prog_##name)
