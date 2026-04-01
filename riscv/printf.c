#include <stdarg.h>
#include <stdint.h>
#include "uart.h"
#include "printf.h"

static const char g_digits_lower[] = "0123456789abcdef";
static const char g_digits_upper[] = "0123456789ABCDEF";

static void putnchar(char ch, int count) {
   while (count-- > 0)
      uart_putc(ch);
}

static int strnlen_simple(const char* s) {
   int n = 0;

   while (s[n] != '\0')
      n++;

   return n;
}

static int u32_to_str(uint32_t val, unsigned base, int upper, char* buf) {
   const char* digits = upper ? g_digits_upper : g_digits_lower;
   int i = 0;

   if (base < 2 || base > 16)
      return 0;

   do {
      buf[i++] = digits[val % base];
      val /= base;
   } while (val != 0);

   return i;
}

static void print_unsigned(uint32_t val, unsigned base, int upper,
                           int width, int zero_pad) {
   char buf[16];
   int len = u32_to_str(val, base, upper, buf);
   int pad = width - len;

   if (pad < 0)
      pad = 0;

   putnchar(zero_pad ? '0' : ' ', pad);

   while (--len >= 0)
      uart_putc(buf[len]);
}

static void print_signed(int32_t val, int width, int zero_pad) {
   uint32_t mag;
   int negative = (val < 0);
   char buf[16];
   int len;
   int pad;

   if (negative)
      mag = (uint32_t)(-(val + 1)) + 1u;
   else
      mag = (uint32_t)val;

   len = u32_to_str(mag, 10, 0, buf);
   pad = width - len - (negative ? 1 : 0);

   if (pad < 0)
      pad = 0;

   if (!zero_pad)
      putnchar(' ', pad);

   if (negative)
      uart_putc('-');

   if (zero_pad)
      putnchar('0', pad);

   while (--len >= 0)
      uart_putc(buf[len]);
}

static void print_string(const char* s, int width) {
   int len;

   if (s == 0)
      s = "(null)";

   len = strnlen_simple(s);

   if (width > len)
      putnchar(' ', width - len);

   uart_puts(s);
}

static void print_pointer(const void* ptr) {
   uintptr_t v = (uintptr_t)ptr;
   int nibbles = (int)(sizeof(uintptr_t) * 2);
   int i;

   uart_putc('0');
   uart_putc('x');

   for (i = nibbles - 1; i >= 0; i--) {
      unsigned d = (unsigned)((v >> (i * 4)) & 0xf);
      uart_putc(g_digits_lower[d]);
   }
}

void vprintf(const char* fmt, va_list ap) {
   while (*fmt) {
      int zero_pad = 0;
      int width = 0;

      if (*fmt != '%') {
         uart_putc(*fmt++);
         continue;
      }

      fmt++;

      if (*fmt == '%') {
         uart_putc('%');
         fmt++;
         continue;
      }

      if (*fmt == '0') {
         zero_pad = 1;
         fmt++;
      }

      while (*fmt >= '0' && *fmt <= '9') {
         width = width * 10 + (*fmt - '0');
         fmt++;
      }

      switch (*fmt) {
         case 'd':
         case 'i':
            print_signed(va_arg(ap, int), width, zero_pad);
            fmt++;
            break;

         case 'u':
            print_unsigned(va_arg(ap, unsigned int), 10, 0, width, zero_pad);
            fmt++;
            break;

         case 'x':
            print_unsigned(va_arg(ap, unsigned int), 16, 0, width, zero_pad);
            fmt++;
            break;

         case 'X':
            print_unsigned(va_arg(ap, unsigned int), 16, 1, width, zero_pad);
            fmt++;
            break;

         case 'p':
            print_pointer(va_arg(ap, const void*));
            fmt++;
            break;

         case 's':
            print_string(va_arg(ap, const char*), width);
            fmt++;
            break;

         case 'c':
            uart_putc((char)va_arg(ap, int));
            fmt++;
            break;

         default:
            uart_putc('%');
            if (*fmt)
               uart_putc(*fmt++);
            break;
      }
   }
}

void printf(const char* fmt, ...) {
   va_list ap;

   va_start(ap, fmt);
   vprintf(fmt, ap);
   va_end(ap);
}
