#include "interrupts.h"

#include "aarch64.h"
#include "context.h"
#include "drivers/timer.h"
#include "printf.h"
#include "proc.h"
#include "scheduler.h"
#include "types.h"
#include "utils.h"


[[noreturn]] void handle_interrupt(context_t* ctx) {
  current_process->context = ctx;

  // NEW:  Check which type of interrupt occurred
  uint32 irq_type = mmio_read32(INT_SOURCE_0);
  switch (irq_type) {
    case (GENERIC_TIMER_INTERRUPT):
      timer_interrupt(); // reset timer, call handle_timer_interrupt (below)
      break;
    default:
      panic("Unknown interrupt type\r\n");
  }

  restore_context(ctx); // should not reach this point
}

// Run this when a timer interrupt occurs!
[[noreturn]] void handle_timer_interrupt() {
  // Finally:  force this process to yield
  // (just like when we implemented the yield function!)
  handle_yield(current_process->context);  // may want a helper called yield() for this
}



// Enable ARM generic timer interrupt controller
void enable_interrupt_controller(void) {
  // . . .
}


void enable_interrupts() {
  // . . .
}
