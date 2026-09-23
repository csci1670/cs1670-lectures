#ifndef _PROC_H
#define _PROC_H

#include "constants.h"
#include "context.h"
#include "limits.h"
#include "types.h"

// Current state of a process:
//   UNUSED: unallocated space in the process table
//   USED: allocated but not runnable yet (currently being prepared to be run)
//   RUNNABLE: ready to run, not on CPU
//   RUNNING: currently on CPU
enum procstate { UNUSED, USED, RUNNABLE, RUNNING };

// Process descriptor: struct that tracks information about running process
struct proc {
  enum procstate state;  // Process state
  // TODO: think about what other fields we need to add here
};

struct proc* allocproc(const char* name);
void print_process_table();

#endif  // _PROC_H
