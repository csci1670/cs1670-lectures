#include "scheduler.h"

#include "context.h"
#include "limits.h"
#include "printf.h"
#include "proc.h"
#include "types.h"
#include "utils.h"

extern struct proc* current_process;
extern struct proc processes[];

// Round-robin starting just after current_process (a process that just gave up
// the CPU is considered last).
// Returns the PID of the next process to run.
// Panics if no process can run.
struct proc* pick_next(void) {
  // TODO: implement!
}

// Restores p's registers on CPU and resumes it.
// `restore_context` returns to userspace or into the kernel depending on
// p's saved SPSR value, which determines the destination via the mode bits.
// This function never returns to the caller. If the process that made a
// syscall or was preempted before entering the kernel is still runnable, it
// will be resumed by another call to `scheduler` in the future.
[[noreturn]] void resume(struct proc* p) {
  current_process = p;
  p->state = RUNNING;
  restore_context(p->context);
}

[[noreturn]] void scheduler() {
  resume(pick_next());
}

// A process calling yield is still runnable, but gives a chance
// for the scheduler to schedule another process on the CPU
// (it could be the same process again).
[[noreturn]] void handle_yield(context_t* ctx) {
  // TODO: implement!
}
