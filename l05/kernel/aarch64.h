#ifndef _AARCH64_H
#define _AARCH64_H

// AArch64 (ARM64) architecture details are defined in this file.
// These target the ARMv8.0-A architecture used by the ARM Cortex-A53 on the
// Raspberry Pi 3B+.
//
// Note that these are *ARM* design decisions, rather than those of the
// Raspberry Pi or I/O peripheral device designers (those addresses and
// register values are defined in `memlayout.h` and in the files in the
// `drivers` directory).
//
// We give you the register values below because working them out bit by bit
// from the ARM Architecture Reference Manual is laborious and not the point of
// the project. Do read the comments, though: each one tells you what the bit
// actually controls, and you will need that understanding.
//

// ---------------------------------------------------------------------------
// System Control Register (SCTLR_EL1)
// ---------------------------------------------------------------------------
// Controls the MMU, the caches, and the endianness of EL1 and EL0. We want all
// of these features off: there are no page tables yet, and caching would get in
// the way of the memory-mapped I/O your drivers do.
#define SCTLR_MMU_DISABLED (0 << 0)      // bit 0: 0 = MMU disabled
#define SCTLR_MMU_ENABLED (1 << 0)       //        1 = MMU enabled
#define SCTLR_D_CACHE_DISABLED (0 << 2)  // bit 2: 0 = data cache disabled
#define SCTLR_I_CACHE_DISABLED \
  (0 << 12)  // bit 12: 0 = instruction cache disabled
#define SCTLR_EOE_LITTLE_ENDIAN \
  (0 << 24)  // bit 24: 0 = EL0 uses Little Endian byte order
#define SCTLR_EE_LITTLE_ENDIAN \
  (0 << 25)  // bit 25: 0 = EL1 uses Little Endian byte order
// Sets a bunch of reserved bits that cover features not implemented on the
// RPi's ARM Cortex-A53 CPU (which uses ARMv8.0-A, which doesn't have these
// features).
#define SCTLR_RESERVED ((3 << 28) | (3 << 22) | (1 << 20) | (1 << 11))
// Compose the above into a single bit pattern to write into SCTLR_EL1.
#define SCTLR_VALUE_MMU_DISABLED                                      \
  (SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_DISABLED | \
   SCTLR_D_CACHE_DISABLED | SCTLR_MMU_DISABLED)

// ---------------------------------------------------------------------------
// Hypervisor Configuration Register (HCR_EL2)
// ---------------------------------------------------------------------------
// Bit 31 (RW) selects the execution state of the exception levels *below* EL2.
// Its reset value selects AArch32, so you must set it before dropping to EL1 --
// otherwise your 64-bit kernel will find itself running as 32-bit code.
//
// Note that this one is written as a plain hex constant rather than as
// `0b1 << 31`: shifting a 1 into bit 31 overflows a signed `int` in C, and the
// result would sign-extend to a 64-bit value with all of the top 32 bits set.
// A `u` suffix would fix that in C, but this header is also included from
// assembly, and GNU as does not accept C integer suffixes -- so hex it is.
#define HCR_RW 0x80000000       // 0 = AArch32, 1 = AArch64
#define HCR_VALUE_EL2 (HCR_RW)  // Everything else in the HCR is set to zeroes.

// ---------------------------------------------------------------------------
// Saved Program Status Register (SPSR)
// ---------------------------------------------------------------------------
// SPSR holds a saved copy of PSTATE: the processor state to restore on an
// exception return (`eret`). The hardware writes it for you when it takes an
// exception; your kernel writes it by hand when it wants to `eret` somewhere of
// its own choosing (dropping from EL2 to EL1, or resuming a saved context).
//
// Bits 0-3 select the exception level to return to, and which stack pointer
// that level will use.
#define SPSR_EL1h \
  (0b101 << 0)  // EL1 with stack pointer in SP_EL1 (kernel stack). This is
                // called "EL1h" in the ARM spec.
#define SPSR_EL2h \
  (0b1001 << 0)  // EL2 with stack pointer in SP_EL2 (hypervisor stack). This is
                 // called "EL2h" in the ARM spec.
#define SPSR_EL0t \
  0b0000  // EL0 with stack pointer in SP_EL0 (user stack).
          // This is called "EL0t" in the ARM spec.
// Mask off everything but the mode bits, to ask "where did this come from?"
#define SPSR_M_MASK 0b1111

// Bits 6-8 mask (disable) exceptions and interrupts. A 1 means "masked".
// (Bit 9, the Debug mask, is left alone: we never take debug exceptions.)
#define SPSR_MASK_FIQ (0b1 << 6)     // Fast Interrupts (FIQ)
#define SPSR_MASK_IRQ (0b1 << 7)     // Interrupts (IRQ) -- the timer, for you
#define SPSR_MASK_SERROR (0b1 << 8)  // System Error (SError) exceptions
#define SPSR_MASK_ALL (0b111 << 6)   // Mask all of the above.

// The PSTATE to set up before the `eret` in your bootup code that leaves EL2:
// resume at EL1 on the kernel stack, with every interrupt masked. Masking
// matters, because at this point in bootup you have not installed an exception
// vector table, so an interrupt would send the CPU to an unknown address.
#define SPSR_VALUE_EL2 (SPSR_EL1h | SPSR_MASK_ALL)
// Value to set PSTATE to when returning to the kernel (e.g., returning to
// continue a blocked syscall):
//   resume at EL1 with the kernel stack (EL1h) + all interrupts masked.
#define SPSR_KERNEL (SPSR_EL1h | SPSR_MASK_ALL)
// Value to set PSTATE to when returning to a user process (eret).
// The kernel never runs with this PSTATE: on every exception entry
//   the hardware sets PSTATE.I=1 (IRQs masked inside the kernel)
// Our initial value masks all interrupts and returns to EL1, but you will
//   need to change this as you implement Project 2.
#define SPSR_PROC (SPSR_EL1h | SPSR_MASK_ALL)

// Exception Syndrome Register (ESR) definitions. This register captures
// information about exceptions that occur, and is used by the kernel to
// determine how to handle the exception.
// We use this information primarily when handling syscalls and to print
// kernel panic messages for unhandled exceptions.
//
// Right shift ESR_EL1 by this many bits to get Exception Class value
#define ESR_EL1_EC_OFFSET 26
// SVC instruction execution in AArch64 state
#define ESR_EL1_EC_SVC_CAUSE 0b010101  // 0001 0101
// To mask off bits other than the EC bits after shift
#define ESR_EL1_EC_MASK 0b111111  // 0011 1111
// To mask off bits other than the imm16 bits (16 bits)
#define ESR_EL1_IMM_MASK 0xffff  // 1111 1111 1111 1111

// SCR_EL3: hand EL2 a non-secure, AArch64 world
#define SCR_RESERVED (0b11 << 4)  // Bits 4-5: reserved (RES1)
#define SCR_RW (0b1 << 10)        // bit 10: EL2 is AArch64
#define SCR_NS (0b1 << 0)         // bit 0: lower ELs are in non-secure mode
#define SCR_VALUE_EL3 (SCR_RESERVED | SCR_RW | SCR_NS)

// SPSR_EL3: eret target state = EL2h, interrupts masked
#define SPSR_VALUE_EL3 (SPSR_MASK_ALL | SPSR_EL2h)

#endif  // _AARCH64_H
