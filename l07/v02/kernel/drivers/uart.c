#include "drivers/uart.h"

#include "drivers/gpio.h"
#include "memlayout.h"
#include "types.h"
#include "utils.h"

void uart_init() {
  gpio_init();

  mmio_write32(UART_CR, 0);       // Disable UART
  mmio_write32(UART_IBRD, 2500);  // Configure UART baud rate, integer part
  mmio_write32(UART_FBRD, 0);     // as above, fractional part
  mmio_write32(
      UART_LCRH,
      (0b1 << 4) | (0b11 << 5));  // Bit 4 enables FIFO (buffering).
                                  // Bits 5:6 configure 8-bit mode (0b11)
  mmio_write32(
      UART_CR,
      0b1 | (0b1 << 8) | (0b1 << 9) |
          (0b1 << 15));  // Bit 0 enables UART.
                         // Bit 8 enables transmit (TXE)
                         // Bit 9 enables receive (RXE)
                         // Bit 15 enables CTS hardware flow control (CTSEN)
}

void uart_send(char c) {
  // Wait until UART transmit FIFO has space
  while (true) {
    // Bit 5 of UART_FR indicates "TX FIFO full" (TXFF), so wait until zero
    if (!(mmio_read32(UART_FR) & (1 << 5))) {
      break;
    }
  }
  mmio_write32(UART_DR, c);
}

void uart_send_string(const char* s) {
  for (uint32 i = 0; s[i] != '\0'; ++i) {
    uart_send((char)s[i]);
  }
}
