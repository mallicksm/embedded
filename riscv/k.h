#ifndef K_H
#define K_H
#include <stdint.h>
#include <stddef.h>

//------------------------------------------------------------------------------
// Kernel core utility functions
//------------------------------------------------------------------------------
// prog.c
void prog_cli(void);
// stdlib.c, string.c
int strcmp(const char* s1, const char* s2);
int atoi(const char*);
unsigned long strtoul(const char* nptr, char** endptr, int base);
// trap.c
void timer_set(uint32_t);
void trap_enable(void);
void trap_handler(void);
// printf.c
void printf(const char* fmt, ...);
// uart.c
void uart_putc(char);
void uart_puts(const char*);
char uart_getc(void);
int uart_getc_nonblock(void);

//------------------------------------------------------------------------------
// Kernel core utilities (minimal, assert-driven, no silent failures)
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
// NOTE:
//   - panic() is the ONLY place that should print failure info
//   - ASSERT/PANIC macros should not duplicate printing
//   - file/line reserved for future (debug bring-up)
//------------------------------------------------------------------------------
static inline void panic(const char* file, int line) {
   uart_puts("panic\n");

   (void)file;
   (void)line;

   for (;;)
      ; // intentional hard stop
}

// Always-fail macro (never call panic() directly)
#define PANIC() panic(__FILE__, __LINE__)

// Invariant enforcement (hardware mindset)
#define ASSERT(x)                   \
   do {                             \
      if (!(x)) {                   \
         panic(__FILE__, __LINE__); \
      }                             \
   } while (0)

// Assert with message (caller must include '\n')
#define ASSERT_MSG(x, msg)          \
   do {                             \
      if (!(x)) {                   \
         uart_puts(msg);            \
         panic(__FILE__, __LINE__); \
      }                             \
   } while (0)

//------------------------------------------------------------------------------
// Utility macros
//------------------------------------------------------------------------------
// Get container struct from member pointer
#define container_of(ptr, type, member) \
   ((type*)((char*)(ptr) - offsetof(type, member)))

// NOTE: 'a' MUST be power-of-2
#define ALIGN_UP(x, a) (((x) + ((a) - 1)) & ~((a) - 1))
#define ALIGN_DOWN(x, a) ((x) & ~((a) - 1))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

//------------------------------------------------------------------------------
// Debug / development helpers
//------------------------------------------------------------------------------
// Unreachable code path (use instead of silent fallthrough)
#define UNREACHABLE() \
   do {               \
      PANIC();        \
   } while (0)

// Intentional infinite loop (NOT an error)
#define SPIN() \
   do {        \
      for (;;) \
         ;     \
   } while (0)

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
// RISC-V CSR access (generic)
//------------------------------------------------------------------------------
// Read CSR
#define CSR_READ(csr)               \
   ({                               \
      uint32_t __tmp;               \
      asm volatile("csrr %0, " #csr \
                   : "=r"(__tmp));  \
      __tmp;                        \
   })

// Write CSR
#define CSR_WRITE(csr, val)                         \
   do {                                             \
      uint32_t __v = (uint32_t)(val);               \
      asm volatile("csrw " #csr ", %0" ::"r"(__v)); \
   } while (0)

// Set bits in CSR
#define CSR_SET(csr, mask)                          \
   do {                                             \
      uint32_t __m = (uint32_t)(mask);              \
      asm volatile("csrs " #csr ", %0" ::"r"(__m)); \
   } while (0)

// Clear bits in CSR
#define CSR_CLEAR(csr, mask)                        \
   do {                                             \
      uint32_t __m = (uint32_t)(mask);              \
      asm volatile("csrc " #csr ", %0" ::"r"(__m)); \
   } while (0)
#endif

//------------------------------------------------------------------------------
// CSR bit definitions
//------------------------------------------------------------------------------

// uart
#define UART0_BASE 0x10000000

#define UART_RBR (*(volatile uint8_t*)(UART0_BASE + 0x00))
#define UART_THR (*(volatile uint8_t*)(UART0_BASE + 0x00))
#define UART_LSR (*(volatile uint8_t*)(UART0_BASE + 0x05))

// mstatus
#define MSTATUS_MIE (1u << 3)

// mie
#define MIE_MTIE (1u << 7)

// mcause
#define MCAUSE_INT (1u << 31)

#define MTIME ((volatile uint64_t*)0x0200BFF8)
#define MTIMECMP ((volatile uint64_t*)0x02004000)
