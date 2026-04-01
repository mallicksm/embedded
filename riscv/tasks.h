#pragma once

#include "proc.h"

struct tasks {
   const char* name;
   struct task* task;
};

#define REGISTER_TASK(_name, _task)          \
   static const struct tasks __tasks_##_task \
      __attribute__((used, section(".tasks"))) = {(_name), &(_task)}

extern const struct tasks __tasks_start[];
extern const struct tasks __tasks_end[];
