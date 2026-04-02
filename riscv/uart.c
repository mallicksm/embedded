#include <stdint.h>
#include "uart.h"

extern char __uart0_base[];

#define UART0_BASE ((uintptr_t)__uart0_base)
#define UART_RBR (*(volatile uint8_t*)(UART0_BASE + 0x00))
#define UART_THR (*(volatile uint8_t*)(UART0_BASE + 0x00))
#define UART_LSR (*(volatile uint8_t*)(UART0_BASE + 0x05))

#define UART_LSR_DR (1 << 0)
#define UART_LSR_THRE (1 << 5)

void uart_putc(char c) {
   if (c == '\n') {
      uart_putc('\r'); // inject CR before LF
   }

   while (!(UART_LSR & (1 << 5))) {
      ; // wait for THR empty
   }

   UART_THR = c;
}

int uart_getc_nonblock(void) {
   if (!(UART_LSR & UART_LSR_DR)) {
      return -1; // no data available
   }
   return UART_RBR;
}

char uart_getc(void) {
   while ((UART_LSR & UART_LSR_DR) == 0) {
   }

   return (char)UART_RBR;
}

void uart_puts(const char* s) {
   while (*s) {
      if (*s == '\n') {
         uart_putc('\r');
      }
      uart_putc(*s++);
   }
}
