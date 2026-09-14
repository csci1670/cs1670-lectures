#include "types.h"
#include "utils.h"

// BCM2837 Physical GPIO Base Address for RPi 3
#define GPIO_BASE 0x3F200000

// Device MMIO Register Addresses -- TODO: find addresses!
#define GPFSEL2 ((volatile uint32*)(GPIO_BASE + 0x0))
#define GPSET0 ((volatile uint32*)(GPIO_BASE + 0x0))
#define GPCLR0 ((volatile uint32*)(GPIO_BASE + 0x0))

void setup();

void blink_main(void) {
  setup();

  while (true) {
    // Set Pin HIGH (Write a 1 to bit 21 of GPSET0)
    *GPSET0 |= (0b1 << 21);
    delay_cycles(1500000);

    // Set Pin LOW (Write a 1 to bit 21 of GPCLR0)
    *GPCLR0 |= (0b1 << 21);
    delay_cycles(1500000);
  }
}

void setup(void) {
  // 1. Set GPIO 21 to Output mode
  // Pin 21 uses bits 3-5 of GPFSEL2. Output mode is binary 001.
  uint32 reg = *GPFSEL2;
  reg &= ~(0b111 << 3);  // Clear bits 3, 4, and 5
  reg |= (0b1 << 3);     // Set bit 3 (Value = 1, meaning Output)
  *GPFSEL2 = reg;
}
