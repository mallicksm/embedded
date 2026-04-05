#pragma once

struct progs {
   const char* name;
   void (*fn)(void);
};

#define REGISTER_PROG(_name, _fn)          \
   static const struct progs __progs_##_fn \
      __attribute__((used, section(".progs"))) = {(_name), (_fn)}

extern const struct progs __progs_start[];
extern const struct progs __progs_end[];

#define PROG_COOP 0
#define PROG_PREE 1
#define DEFINE_PROG(name, type)                         \
   void prog_##name(void) {                             \
      for (;;) {                                        \
         printf("Task (%s):" #name "\n",                \
                (type) == PROG_COOP ? "coop" : "pree"); \
         if ((type) == PROG_COOP) {                     \
            task_yield();                               \
         }                                              \
      }                                                 \
   }                                                    \
   REGISTER_PROG(#name, prog_##name)
