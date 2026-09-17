#include "memlayout.h"
#include "types.h"
#include "utils.h"

#define GPIO_BASE (PERIPHERALS_BASE + 0x200000)
#define GPFSEL2 ((volatile uint32*)(GPIO_BASE + 0x08))  // Pin 21 is in GPFSEL2
#define GPSET0 ((volatile uint32*)(GPIO_BASE + 0x1C))   // Pins 0-31
#define GPCLR0 ((volatile uint32*)(GPIO_BASE + 0x28))   // Pins 0-31

#define PIN 20

// Initializes buzzer
void beep_init(void) {
  uint32 reg = *GPFSEL2;
  reg &= ~(0b111 << 0);  // Clear bits 0-2 for pin 20
  reg |= (0b1 << 0);     // Set bit 0 (value = 1, meaning output)
  *GPFSEL2 = reg;
}

// Buzzes for a short duration
void beep(void) {
  // Beep!
  *GPSET0 |= (1 << PIN);
  delay_cycles(2000000);
  *GPCLR0 |= (1 << PIN);
}

