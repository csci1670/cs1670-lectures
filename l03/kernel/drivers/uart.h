#ifndef _DRIVERS_UART_H
#define _DRIVERS_UART_H

#include "memlayout.h"
#include "types.h"

// PL011 UART (p. 175ff, §13; note errata)
//
// TODO: You'll need to change all offsets below from 0x0 to their true
// value (0x0 is a placeholder). We also only give you the UART_BASE
// constant and one example register (UART_DR). You will need to fill in
// the entries for the necessary registers and their offsets based on the
// information in the BCM2835 ARM Peripherals datasheet or the PL011 UART
// datasheet.
#define UART_BASE (PERIPHERALS_BASE + 0x0)
#define UART_DR (UART_BASE + 0x0)  // data register
// #define UART_XXX (UART_BASE + TODO)    // ... other registers

// Declare public functions like `uart_init` here!

#endif  // _DRIVERS_UART_H
