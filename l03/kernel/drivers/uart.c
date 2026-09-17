#include "drivers/uart.h"

#include "drivers/gpio.h"
#include "types.h"
#include "utils.h"

void uart_init() {
  gpio_init();

  // TODO
}

void uart_send(char c) {
  // TODO
}

void uart_send_string(const char* s) {
  for (uint32 i = 0; s[i] != '\0'; ++i) {
    uart_send((char)s[i]);
  }
}
