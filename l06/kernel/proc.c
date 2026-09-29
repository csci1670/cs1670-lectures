#include "proc.h"

#include "limits.h"
#include "printf.h"
#include "utils.h"

struct proc processes[NPROC];

struct proc* current_process;

struct proc* allocproc(const char* name) {
  // TODO: implement to allocate a new process and descriptor for it!
  return nullptr;
}

void print_process_table() {
  printf("\r\nACTIVE PROCESSES:\r\n");
  for (uint64 i = 0; i < NPROC; i++) {
    struct proc* p = &processes[i];
    enum procstate state = p->state;
    if (state != UNUSED) {
      switch (state) {
        case USED:
          printf("Process %u: USED\r\n", p->pid);
          break;
        case RUNNABLE:
          printf("Process %u: RUNNABLE (%s)", p->pid, p->name);
          break;
        case RUNNING:
          printf("Process %u: RUNNING (%s)", p->pid, p->name);
          break;
        default:
          panic("Unknown process state %u", p->state);
          break;
      }
      printf("\r\n");
    }
  }
}
