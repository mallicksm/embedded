#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include "uart.h"

static const char g_digits_lower[] = "0123456789abcdef";
static const char g_digits_upper[] = "0123456789ABCDEF";

enum fmt_len {
   LEN_DEFAULT,
   LEN_CHAR,
   LEN_SHORT,
   LEN_LONG,
   LEN_LLONG,
   LEN_SIZE,
   LEN_PTRDIFF,
};

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

static int ull_to_str(unsigned long long val, unsigned base, int upper, char* buf) {
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

static void print_unsigned(unsigned long long val, unsigned base, int upper,
                           int width, int zero_pad) {
   char buf[32];
   int len = ull_to_str(val, base, upper, buf);
   int pad = width - len;

   if (pad < 0)
      pad = 0;

   putnchar(zero_pad ? '0' : ' ', pad);

   while (--len >= 0)
      uart_putc(buf[len]);
}

static void print_signed(long long val, int width, int zero_pad) {
   unsigned long long mag;
   int negative = (val < 0);
   char buf[32];
   int len;
   int pad;

   if (negative)
      mag = (unsigned long long)(-(val + 1)) + 1;
   else
      mag = (unsigned long long)val;

   len = ull_to_str(mag, 10, 0, buf);
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

   uart_putc('0');
   uart_putc('x');

   for (int i = nibbles - 1; i >= 0; i--) {
      unsigned d = (unsigned)((v >> (i * 4)) & 0xf);
      uart_putc(g_digits_lower[d]);
   }
}

static unsigned long long get_unsigned_arg(va_list ap, enum fmt_len len) {
   switch (len) {
      case LEN_CHAR:
         return (unsigned char)va_arg(ap, unsigned int);
      case LEN_SHORT:
         return (unsigned short)va_arg(ap, unsigned int);
      case LEN_LONG:
         return va_arg(ap, unsigned long);
      case LEN_LLONG:
         return va_arg(ap, unsigned long long);
      case LEN_SIZE:
         return va_arg(ap, size_t);
      case LEN_PTRDIFF:
         return (unsigned long long)va_arg(ap, ptrdiff_t);
      default:
         return va_arg(ap, unsigned int);
   }
}

static long long get_signed_arg(va_list ap, enum fmt_len len) {
   switch (len) {
      case LEN_CHAR:
         return (signed char)va_arg(ap, int);
      case LEN_SHORT:
         return (short)va_arg(ap, int);
      case LEN_LONG:
         return va_arg(ap, long);
      case LEN_LLONG:
         return va_arg(ap, long long);
      case LEN_SIZE:
         return (long long)va_arg(ap, size_t);
      case LEN_PTRDIFF:
         return va_arg(ap, ptrdiff_t);
      default:
         return va_arg(ap, int);
   }
}

void uart_vprintf(const char* fmt, va_list ap) {
   while (*fmt) {
      int zero_pad = 0;
      int width = 0;
      enum fmt_len len = LEN_DEFAULT;

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

      if (*fmt == 'h') {
         fmt++;
         if (*fmt == 'h') {
            len = LEN_CHAR;
            fmt++;
         } else {
            len = LEN_SHORT;
         }
      } else if (*fmt == 'l') {
         fmt++;
         if (*fmt == 'l') {
            len = LEN_LLONG;
            fmt++;
         } else {
            len = LEN_LONG;
         }
      } else if (*fmt == 'z') {
         len = LEN_SIZE;
         fmt++;
      } else if (*fmt == 't') {
         len = LEN_PTRDIFF;
         fmt++;
      }

      switch (*fmt) {
         case 'd':
         case 'i':
            print_signed(get_signed_arg(ap, len), width, zero_pad);
            fmt++;
            break;

         case 'u':
            print_unsigned(get_unsigned_arg(ap, len), 10, 0, width, zero_pad);
            fmt++;
            break;

         case 'x':
            print_unsigned(get_unsigned_arg(ap, len), 16, 0, width, zero_pad);
            fmt++;
            break;

         case 'X':
            print_unsigned(get_unsigned_arg(ap, len), 16, 1, width, zero_pad);
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
   uart_vprintf(fmt, ap);
   va_end(ap);
}
