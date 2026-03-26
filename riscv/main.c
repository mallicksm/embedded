#include <stdint.h>

#define UART0_BASE 0x10000000UL
#define UART_THR   (*(volatile uint8_t *)(UART0_BASE + 0x00))
#define UART_LSR   (*(volatile uint8_t *)(UART0_BASE + 0x05))
#define UART_LSR_THRE (1 << 5)

static void uart_putc(char c) {
   while ((UART_LSR & UART_LSR_THRE) == 0) {
   }

   UART_THR = (uint8_t)c;
}

static void uart_puts(const char *s) {
   while (*s) {
      if (*s == '\n') {
         uart_putc('\r');
      }
      uart_putc(*s++);
   }
}

int main(void) {
   uart_puts("Hello from RV32 bare metal clang on QEMU virt\n");

   for (;;) {
   }

   return 0;
}
