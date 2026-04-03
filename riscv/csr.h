#ifndef CSR_H
#define CSR_H

#include <stdint.h>

#define csr_read(reg) ({                      \
   uint32_t __tmp;                            \
   asm volatile("csrr %0, " #reg              \
                : "=r"(__tmp));               \
   __tmp;                                     \
})

#define csr_write(reg, val)                   \
   asm volatile("csrw " #reg ", %0"           \
                :: "r"(val))

#define csr_set(reg, bits)                    \
   asm volatile("csrs " #reg ", %0"           \
                :: "r"(bits))

#define csr_clear(reg, bits)                  \
   asm volatile("csrc " #reg ", %0"           \
                :: "r"(bits))

#endif