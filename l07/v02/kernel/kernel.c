#include "context.h"
#include "drivers/uart.h"
#include "drivers/timer.h"
#include "exceptions.h"
#include "init.h"
#include "interrupts.h"
#include "printf.h"

void klib_init() {
  *(int (**)(const char*, va_list))F_VPRINTF = vprintf_kernel;
  *(void (**)(void))F_YIELD = yield_entry;
}

void kernel_main() {
  uart_init();
  klib_init();
  exception_init(exception_vector_table);
  timer_init(handle_timer_interrupt); // call this function (in interrupts.c) on each interrupt
  enable_interrupt_controller();

  enable_interrupts();

  printf("Hello world from DemoOS!\r\n");

  init();
}
