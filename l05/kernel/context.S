#include "aarch64.h"
#include "constants.h"

// Restore a saved context (ARMv8-A AArch64).

.section ".text"

.global save_context
.global restore_context

// Save the register values as a context_t in the frame at [sp].
// IMPORTANT: save_context assumes that caller has already adjusted the
// stack pointer (sp) to make room for the context_t, and that the caller
// has already saved fp (x29) and lr (x30) at sp + 0 and sp + 8.
// We do this because the `bl` instruction that calls save_context will
// overwrite x30 (lr) and x29 (fp) with the return address and the current
// frame pointer, so we need to save them before calling save_context in,
// e.g., exception handlers.
//
// While on exception entry the hardware has already set ELR_EL1 (return PC)
// and SPSR_EL1 (saved PSTATE); we need to copy them into the frame
// so they survive a context switch.
save_context:
  // Store General Purpose Registers
  stp x0,  x1,  [sp, #16 * 1] // store x0 and x1 at sp + 0 bytes
  // TODO: handle other registers, including system registers!

  ret

// [[noreturn]] void restore_context(context_t* c)
//
// Loads previously stored context on the CPU registers fron *c, then eret
// SPSR_EL1 holds whether to return to user (EL0t) or resume kernel (EL1h).
// After this returns, SP_EL1 = c + S_FRAME_SIZE
//   --> the empty top of the kernel stack for a user trapframe,
//   --> the kernel stack pointer at block time for a kernel continuation.
restore_context:
  // TODO: handle other registers, including system registers!
  ldp     x0,  x1,  [x0, #16 * 1] // finally, restore x0 (which we needed up to here)

  eret  // jump to ELR_EL1, restoring PSTATE from SPSR_EL1


// void yield_entry(void)
// Called by a process as an ordinary function (through F_YIELD).
    .globl yield_entry
yield_entry:
    msr  DAIFSET, #2            // 1. mask IRQs: the kernel runs uninterruptibly
    msr  ELR_EL1, x30           // 2. resume address = our return address
    mov  x9, #SPSR_PROC         // 3. processor state to resume the process with
    msr  SPSR_EL1, x9           //    (uses x9 as a scratch register; see below)
    sub  sp, sp, #S_FRAME_SIZE  // 4. make room for the context on the stack
    stp  fp, lr, [sp]           // 5. save fp/lr now: `bl` is about to clobber lr
    bl   save_context           // 6. save everything else
    mov  x0, sp                 // 7. arg0 = pointer to the saved context
    bl   handle_yield           //    ...and into C, never to return
    b    .                      //    (should be unreachable)
