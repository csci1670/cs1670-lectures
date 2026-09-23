#ifndef _CONTEXT_H
#define _CONTEXT_H

#include "constants.h"
#include "types.h"

// The full set of register of a process.
// A context can represent:
//   - a trapframe: a user proc state saved on exception entry (spsr_el1 = EL0t)
//     restored by `eret` to return to user;
//   - a process blocked mid-syscall (spsr_el1 = EL1h), restored by `eret`
//     to resume kernel code (see context.S:context_switch).
typedef struct context {
  uint64 fp;        // x29 = frame pointer (fp)
  uint64 lr;        // x30 = link register (lr)
  uint64 regs[29];  // x0..x28
  uint64 sp_el0;    // user stack pointer
  uint64 elr_el1;   // resume PC (user PC for a trapframe, kernel PC otherwise)
  uint64 spsr_el1;  // saved PSTATE (selects EL0t vs. EL1h on eret)
} context_t;

// Ensures that the manually declared size of context matches this declaration
_Static_assert(sizeof(context_t) == S_FRAME_SIZE,
               "context_t must be S_FRAME_SIZE");

[[noreturn]] void yield_entry();
[[noreturn]] void restore_context(context_t* c);

#endif  // _CONTEXT_H
