// Raspberry Pi 3B (BCM2837 SoC) physical memory layout,
// as defined in official documentation:
// https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
//
// except for the following differences:
// - RPi 3 uses a peripheral base address of 0x3F'000'00 rather than
//   0x20'000'000

#ifndef _MEMLAYOUT_H
#define _MEMLAYOUT_H

// Location of the initial kernel stack in physical memory.
// Must be 16-byte aligned.
#define INITIAL_KERNEL_STACK (1 << 22)

// Peripherals are mapped starting at 0x3F00'0000, giving 1 MB for I/O memory
// Note that the documentation lists addresses starting at 0x7E00'0000. This is
// where the actual ARM hardware has these peripherals, but they are mapped into
// the SoC's address space at 0x3F00'0000
#define PERIPHERALS_BASE 0x3F000000
#define PERIPHERALS_LIMIT 0x3FFFFFFF

// Trampoline to user-reachable functions (like yield or printf).
// This is essentially an array of function pointers at a known location, so
// that programs can use these functions as a library.
//
// The pointers get initialized in `kernel_main` (kernel.c) to point to the
// actual machine code. User programs can then call kernel functions by simply
// following those pointers (even though they weren't compiled together with
// the kernel).
#define F_BASE 0x1000000
#define F_VPRINTF (F_BASE - sizeof(void*))

#endif  // _MEMLAYOUT_H
