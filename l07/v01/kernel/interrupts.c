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
  // ***NOTE***:  this version assumes that all interrupts are timer interrupts
  // In our OS, this will not be true!!!
  // See your project handout for details on how to set up a generic interrupt
  // handler, which can service multiple types of interrupts.

  timer_reset(); // Prepare timer hardware for next interrupt

  // Finally:  force this process to yield
  // (just like when we implemented the yield function!)
  current_process->context = ctx;
  handle_yield(ctx);

  // Note:  you'll probably want a helper yield() that yields the current process
}


// Enable ARM generic timer interrupt controller
void enable_interrupt_controller(void) {
  // . . .
}


void enable_interrupts() {
  // . . .
}
