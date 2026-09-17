#include "drivers/gpio.h"

#include "utils.h"

void gpio_init() {
  // Select the right alternative function for each pin we're using
  unsigned int selector = mmio_read32(GPFSEL1);
  selector &= ~(0b111 << 12);  // clear bits for pin 14
  selector |= 0b100 << 12;     // set pin 14 to ALT0 functionality (TXD0)
  selector &= ~(0b111 << 15);  // clear bits for pin 15
  selector |= 0b100 << 15;     // set pin 15 to ALT0 functionality (RXD0)
  selector &= ~(0b111 << 18);  // clear bits for pin 16
  selector |= 0b111 << 18;     // set pin 16 to ALT3 functionality (CTS0)
  mmio_write32(GPFSEL1, selector);

  // Enable the GPIO pins
  mmio_write32(GPPUD, 0);      // disable pull-up/down for pins 14, 15, and 16
  delay_cycles(150);
  // enable clock for pins 14, 15, and 16; a clock signal is necessary so that
  // the configuration change actually gets applied
  mmio_write32(GPPUDCLK0, (0b1 << 14) | (0b1 << 15) | (0b1 << 16));
  delay_cycles(150);
  mmio_write32(GPPUDCLK0, 0);  // disable clock again
}
