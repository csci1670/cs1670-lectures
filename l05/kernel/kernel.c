#include "context.h"
#include "drivers/uart.h"
#include "init.h"
#include "printf.h"

void klib_init() {
  *(int (**)(const char*, va_list))F_VPRINTF = vprintf_kernel;
  *(void (**)(void))F_YIELD = yield_entry;
}

void kernel_main() {
  uart_init();
  klib_init();

  printf("Hello world from DemoOS!\r\n");

  init();
}
