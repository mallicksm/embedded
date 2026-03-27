#include <stdint.h>

extern char __uart0_base[];

#define UART0_BASE ((uintptr_t)__uart0_base)

#define UART_DR (*(volatile uint32_t*)(UART0_BASE + 0x00))
#define UART_FR (*(volatile uint32_t*)(UART0_BASE + 0x18))

#define UART_FR_RXFE (1 << 4) // RX FIFO empty
#define UART_FR_TXFF (1 << 5) // TX FIFO full

#define CMD_BUF_SIZE 128

static void uart_putc(char c) {
   while (UART_FR & UART_FR_TXFF) {
   }

   UART_DR = (uint32_t)c;
}

static char uart_getc(void) {
   while (UART_FR & UART_FR_RXFE) {
   }

   return (char)(UART_DR & 0xFF);
}

static void uart_puts(const char* s) {
   while (*s) {
      if (*s == '\n') {
         uart_putc('\r');
      }
      uart_putc(*s++);
   }
}

static void uart_prompt(void) {
   uart_puts("> ");
}

int main(void) {
   char buf[CMD_BUF_SIZE];
   int idx = 0;

   uart_puts("Hello from AArch64 bare metal clang on QEMU virt\n");
   uart_puts("Type a command and press Enter.\n");
   uart_prompt();

   for (;;) {
      char c = uart_getc();

      if (c == '\r' || c == '\n') {
         uart_puts("\n");
         buf[idx] = '\0';

         uart_puts("You typed: ");
         uart_puts(buf);
         uart_puts("\n");

         idx = 0;
         uart_prompt();
      } else if (c == 0x7f || c == 0x08) {
         if (idx > 0) {
            idx--;
            uart_putc('\b');
            uart_putc(' ');
            uart_putc('\b');
         }
      } else {
         if (idx < (CMD_BUF_SIZE - 1)) {
            buf[idx++] = c;
            uart_putc(c);
         }
      }
   }

   return 0;
}
