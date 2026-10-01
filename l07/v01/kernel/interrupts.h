#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

#include "memlayout.h"
#include "proc.h"
#include "types.h"

// BCM 2837 data sheet, §7.5 (p. 112)
// . . .
//
// Core 0 ARM local timer interrupt control
#define GENERIC_TIMER_INT_CTRL_0 (ARM_LOCAL_PERIPHERALS_BASE + 0x40)
// Core 0 ARM interrupt control
#define INT_SOURCE_0 (ARM_LOCAL_PERIPHERALS_BASE + 0x60)
#define GENERIC_TIMER_INTERRUPT (0b1 << 1)

void enable_interrupt_controller();


// IRQ entry. Never returns: either resumes the process or schedules another.
[[noreturn]] void handle_interrupt(context_t* tf);


// enables interrupts
void enable_interrupts();
// disables interrupts and returns the current interrupt state
unsigned long disable_interrupts();
// restores a previously saved interrupt state
void restore_interrupts(unsigned long);

// panic for an unhandled exception, printing some debug info
[[noreturn]] void panic_unhandled_exception(uint64 sp);


#endif  // _INTERRUPTS_H
