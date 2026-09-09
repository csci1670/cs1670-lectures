#ifndef _DRIVERS_GPIO_H
#define _DRIVERS_GPIO_H

#include "memlayout.h"

// GPIO pin configuration registers (p. 90ff, §6.1)
//
// TODO: all of these are set to PERIPHERALS_BASE + 0x0; you will need to
// adjust this with the correct offsets determined from the BCM2835 ARM
// Peripherals datasheet.

// GPIO Function Select Registers (GPFSELn): these select what function is
// mapped to each GPIO pin.
// See table on p. 102 and p. 91ff of datasheet.
#define GPFSEL0 (PERIPHERALS_BASE + 0x0)  // pins 0-9
#define GPFSEL1 (PERIPHERALS_BASE + 0x0)  // pins 10-19

// GPIO Pin Up/Down Enable Register (GPPUD): this controls whether pins are
// enabled for "pull-up" or "pull-down", which sets the default voltage level
// of the pin.
// See p. 100f, §6.1 of datasheet.
#define GPPUD (PERIPHERALS_BASE + 0x0)
// GPIO Pin Up/Down Enable Clock Registers (GPPUDCLKn): these control the
// actualy pull-up/pull-down state of the pins, based on the value set in GPPUD.
// Make sure to read the text on p. 101 (§6.1) of the datasheet to understand
// how to use GPUUD and GPPUDCLKn together correctly.
#define GPPUDCLK0 (PERIPHERALS_BASE + 0x0)

#endif  // _DRIVERS_GPIO_H
