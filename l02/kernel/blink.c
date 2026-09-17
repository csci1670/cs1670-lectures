#include "types.h"
#include "utils.h"
#include "blink.h"

#define DEVICE_BASE 0x3F000000

// Device MMIO Register Addresses -- TODO: find addresses!
#define GPFSEL2 ((volatile uint32*)(DEVICE_BASE + 0x200008))
#define GPSET0 ((volatile uint32*)(DEVICE_BASE + 0x20001c))
#define GPCLR0 ((volatile uint32*)(DEVICE_BASE + 0x200028))

void blink_main(void) {
  setup();

  while (true) {
    *GPSET0 |= (0b1 << 21); // Write 1 to bit 21 of GPSET0, sets pin 21 HIGH
    delay_cycles(1500000);

    *GPCLR0 |= (0b1 << 21); // Write 1 to bit 21 of GPCLR0, sets pin 21 LOW
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
