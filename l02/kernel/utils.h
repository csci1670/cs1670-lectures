#ifndef _UTILS_H
#define _UTILS_H

#include "types.h"

// Memory-mapped I/O helper macros for operating on 32-bit memory-mapped I/O
// "registers". `mmio_register32` returns a pointer to the register,
// `mmio_read32` reads the value of the register, and `mmio_write32` writes
// a value to the register.
//
// The cast to `volatile` is important because it tells the compiler that the
// value in the memory address can change at any time (since the address is
// wired to hardware device, rather than actual memory). This also prevents
// the compiler from optimizing away reads or writes to the MMIO register.
#define mmio_register32(reg) ((volatile uint32*)(reg))
#define mmio_read32(reg) (*(mmio_register32(reg)))
#define mmio_write32(reg, v) (*(mmio_register32(reg)) = (v))

// Spin the CPU for at least `ctr` cycles. Returns the number of cycles
// remaining (should be 0).
unsigned int delay_cycles(uint64 ctr);

// Copy n bytes from memory at `src` into memory at `dest`
// (works with overlapping memory).
void* memmove(void* dest, const void* src, uint64 n);

// Wrapper around memmove that assumes the memory regions do not overlap.
// Provides the familiar `memcpy` interface.
void* memcpy(void* dest, const void* src, uint64 n);

// Fills the first n bytes of the memory pointed by src with the constant byte c
void* memset(void* src, int c, uint64 n);

#endif  // _UTILS_H
