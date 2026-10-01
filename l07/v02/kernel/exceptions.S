#include "constants.h"
#include "aarch64.h"

  .section ".text"
  .global save_context
  .global restore_context

  .globl exception_init
exception_init:
  msr VBAR_EL1, x0
  ret

  .globl exception_vector_table
  .balign 0x800


exception_vector_table:
// Section 1:  exceptions taken in current EL, when SP_EL0 is used
  b .              // Synchronous exceptions
  .balign 0x80
  b .              // Interrupts (IRQ, also called "asynchronous exceptions")
  .balign 0x80
  b .              // Fast interrupts (FIQ)
  .balign 0x80
  b .              // System error (SError) exceptions

// Section 2:  exceptions taken in current EL, when using SP_ELx, x > 0
  .balign 0x80
  b .
  .balign 0x80
  b irq_entry  // On IRQ, run our assembly code, which saves context (below)
  .balign 0x80
  b .
  .balign 0x80
  b .
// Section 3:  exceptions taken from level below current EL
  .balign 0x80
  b .           // Unused for now
  .balign 0x80
  b .           // Unused for now
  .balign 0x80
  b .
  .balign 0x80
  b .

// Section 4:  exceptions in 32-bit mode (AArch32) from lower exception level (unused for us)
  .balign 0x80
  b .
  .balign 0x80
  b .
  .balign 0x80
  b .
  .balign 0x80
  b .

// irq_entry:  IRQ entry point  (called handle_el1_irq in lecture)
// this is similar to the yield_entry function we wrote for yield,
// but now adapted for IRQs:  before we can invoke our scheduler (C code),
// we need some assembly code to save our context on the stack
irq_entry:
  sub sp, sp, #S_FRAME_SIZE // allocate the context_t on the stack
  stp fp, lr, [sp]          // save frame pointer and link register before `bl` changes them
  bl save_context           // save our context

  mov x0, sp               // pass context as argument
  bl handle_interrupt      // jump to C function that will use the context
  b .
