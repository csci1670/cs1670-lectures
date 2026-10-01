#include "aarch64.h"
#include "memlayout.h"

    .section ".text.boot"
      .globl _start

_start:
    mrs x0, MPIDR_EL1  // get current core ID into x0
    and x0, x0, #0xFF  // mask out upper bits
    cbz x0, processor0_el3 // jump if we are on core 0
    b loop

// Only processor 0 runs this code
processor0_el3:
    // Switch from EL3 to EL2
    ldr  x0, =SCR_VALUE_EL3  // Set values in System Control Register; see aarch64.h.
                             // NS=1 (non-secure), RW=1 (EL2 runs AArch64) + RES1
    msr  SCR_EL3, x0
    ldr  x0, =SPSR_VALUE_EL3 // Set values in the Saved Program Status Register (SPSR); see aarch64.h.
                             // return to EL2h, DAIF masked (no interrupts)
    msr  SPSR_EL3, x0
    adr  x0, processor0_el2
    msr  ELR_EL3, x0         // Store the address of processor0_el2 into the exception link register
                             // for EL3, meaning that `eret` will use it as the return address and
                             // jump to it.
    eret                     // EL3 -> EL2, lands at el2_to_el1


processor0_el2:
    ldr   x0, =SCTLR_VALUE_MMU_DISABLED
    msr   SCTLR_EL1, x0  // Set values in System Control Register; see aarch64.h.

    ldr   x0, =HCR_VALUE_EL2
    msr   HCR_EL2, x0    // Set values in Hypervisor Configuration Register (HCR); see aarch64.h.

    // Switch from EL2 to EL1
    ldr   x0, =SPSR_VALUE_EL2
    msr   SPSR_EL2, x0   // Set values in the Saved Program Status Register (SPSR); see aarch64.h.

    adr   x0, el1_entry
    msr   ELR_EL2, x0    // Store the address of el1_entry into the exception link register
                         // for EL2, meaning that `eret` will use it as the return address
                         // and jump to it.

    eret                 // Exception return; lowers the exception level from EL2 to EL1 and jumps
                         // to address in ELR_EL2.

el1_entry:
    adr   x0, bss_begin  // Store start address of .bss into x0.
    adr   x1, bss_end    // Store end address of .bss into x1.
    sub   x1, x1, x0     // Calculate length of .bss section as len = bss_end - bss_begin.
    bl    memzero        // Call memzero(bss_begin, len) to clear .bss section.

    mov   sp, #INITIAL_KERNEL_STACK  // Set sp register to point to kernel stack at 0x40'0000; see mm.h.
    mov   x0, #0         // Set processor_id argument to kernel_main, passed in register x0.
    bl    kernel_main    // Jump to kernel C code.

loop:
    b loop
