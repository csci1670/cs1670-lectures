#include "drivers/uart.h"
#include "types.h"
#include "utils.h"

/* Helper to print a null-terminated string */
static void print_str(const char *s) {
  if (!s) {
    s = "(null)";
  }
  while (*s) {
    uart_send(*s++);
  }
}

/* Helper to print unsigned integers in any base (e.g., 10 or 16) */
static void print_unsigned(unsigned long long val, int base, int uppercase) {
  char buf[32];
  int i = 0;
  const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

  if (val == 0) {
    uart_send('0');
    return;
  }

  while (val > 0) {
    buf[i++] = digits[val % base];
    val /= base;
  }

  /* Print characters in reverse order */
  while (--i >= 0) {
    uart_send(buf[i]);
  }
}

/* Helper to print signed integers */
static void print_signed(long long val, int base) {
  if (val < 0) {
    uart_send('-');
    print_unsigned((unsigned long long)(-val), base, 0);
  } else {
    print_unsigned((unsigned long long)val, base, 0);
  }
}

/* Core formatting engine */
int vprintf_kernel(const char *format, va_list args) {
  int count = 0;
  for (const char *p = format; *p != '\0'; p++) {
    if (*p != '%') {
      uart_send(*p);
      count++;
      continue;
    }

    p++;  // Skip past '%'
    if (*p == '\0') break;

    switch (*p) {
      case 'd': {
        long long val = va_arg(args, int);
        print_signed(val, 10);
        break;
      }
      case 'u': {
        unsigned int val = va_arg(args, unsigned int);
        print_unsigned(val, 10, 0);
        break;
      }
      case 'x': {
        unsigned int val = va_arg(args, unsigned int);
        print_unsigned(val, 16, 0);
        break;
      }
      case 'p': {
        void *ptr = va_arg(args, void *);
        print_str("0x");
        print_unsigned((unsigned long long)(uintptr_t)ptr, 16, 0);
        break;
      }
      case 'c': {
        /* Characters are promoted to int when passed through varargs */
        char c = (char)va_arg(args, int);
        uart_send(c);
        break;
      }
      case 's': {
        const char *s = va_arg(args, const char *);
        print_str(s);
        break;
      }
      case '%': {
        uart_send('%');
        break;
      }
      default:
        /* Fallback for unsupported or malformed modifiers */
        uart_send('%');
        uart_send(*p);
        break;
    }
  }
  return count;
}

int printf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  int done = vprintf_kernel(format, args);
  va_end(args);
  return done;
}

int panic(const char *format, ...) {
  uart_send_string("KERNEL PANIC: ");

  va_list args;
  va_start(args, format);
  int done = vprintf_kernel(format, args);
  va_end(args);
  return done;
}
