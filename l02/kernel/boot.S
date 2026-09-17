#include "memlayout.h"

    .section ".text.boot"
      .globl _start

_start:
    mrs x0, MPIDR_EL1  // get current core ID into x0
    and x0, x0, #0xFF  // mask out upper bits
    cbz x0, processor0 // jump if we are on core 0
    b loop

processor0:
    adr x0, bss_begin  // save address of .bss start in x0
    adr x1, bss_end    // save address of .bss end in x1
    sub x1, x1, x0     // compute len of .bss segment by x1 - x0
    bl memzero         // zero out the .bss segment of global variables
    mov sp, 0x400000   // set up our initial stack pointer
    bl pi_main         // jump into C code

loop:
    b loop
