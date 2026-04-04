#ifndef K_H
#define K_H

//------------------------------------------------------------------------------
// Kernel core utilities (minimal, assert-driven, no silent failures)
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Basic types
//------------------------------------------------------------------------------
#include <stdint.h>
#include <stddef.h>
#include "uart.h"

//------------------------------------------------------------------------------
// Panic / assert
//------------------------------------------------------------------------------
// NOTE:
//   - panic() is the ONLY place that should print failure info
//   - ASSERT/PANIC macros should not duplicate printing
//   - file/line reserved for future (debug bring-up)
static inline void panic(const char* file, int line) {
   uart_puts("panic\n");

   (void)file;
   (void)line;

   for (;;);   // intentional hard stop
}

// Always-fail macro (never call panic() directly)
#define PANIC() panic(__FILE__, __LINE__)

// Invariant enforcement (hardware mindset)
#define ASSERT(x)                   \
   do {                            \
      if (!(x)) {                  \
         panic(__FILE__, __LINE__);\
      }                            \
   } while (0)

// Assert with message (caller must include '\n')
#define ASSERT_MSG(x, msg)          \
   do {                            \
      if (!(x)) {                  \
         uart_puts(msg);           \
         panic(__FILE__, __LINE__);\
      }                            \
   } while (0)

//------------------------------------------------------------------------------
// Branch prediction hints (optional)
//------------------------------------------------------------------------------
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

//------------------------------------------------------------------------------
// Utility macros
//------------------------------------------------------------------------------

// Get container struct from member pointer
#define container_of(ptr, type, member) \
   ((type*)((char*)(ptr) - offsetof(type, member)))

// NOTE: 'a' MUST be power-of-2
#define ALIGN_UP(x, a)   (((x) + ((a) - 1)) & ~((a) - 1))
#define ALIGN_DOWN(x, a) ((x) & ~((a) - 1))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

//------------------------------------------------------------------------------
// Debug / development helpers
//------------------------------------------------------------------------------

// Unreachable code path (use instead of silent fallthrough)
#define UNREACHABLE() \
   do { PANIC(); } while (0)

// Intentional infinite loop (NOT an error)
#define SPIN() \
   do { for (;;); } while (0)

//------------------------------------------------------------------------------
// Minimal character helpers
//------------------------------------------------------------------------------
static inline int is_space(char c) {
   return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

static inline int is_digit(char c) {
   return (c >= '0' && c <= '9');
}

static inline int is_hex(char c) {
   return (c >= '0' && c <= '9') ||
          (c >= 'a' && c <= 'f') ||
          (c >= 'A' && c <= 'F');
}

//------------------------------------------------------------------------------
// Kernel conventions (VERY IMPORTANT — your style)
//------------------------------------------------------------------------------
//
//   - No defensive checks for invariants
//   - ASSERT() for "must be true"
//   - PANIC() for fatal paths
//   - SPIN() for intentional infinite loop
//
//   Examples:
//      ASSERT(t != 0);          // invariant
//      PANIC();                 // fatal error
//      SPIN();                  // intentional idle
//
//   Avoid:
//      if (t == 0) return 0;    // ❌ hides bugs
//      while (t != 0)           // ❌ defensive style
//
//------------------------------------------------------------------------------

#endif
