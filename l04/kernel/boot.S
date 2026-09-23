#include "memlayout.h"

    .section ".text.boot"
      .globl _start

_start:
    mrs x0, MPIDR_EL1  // get current core ID into x0
    and x0, x0, #0xFF  // mask out upper bits
    cbz x0, processor0 // jump if we are on core 0
    b loop

processor0:
    adr x0, bss_begin
    adr x1, bss_end
    sub x1, x1, x0
    bl memzero
    mov sp, 0x400000
    bl kernel_main

loop:
    b loop
