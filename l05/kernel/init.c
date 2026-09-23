#include "aarch64.h"
#include "elf.h"
#include "limits.h"
#include "memlayout.h"
#include "printf.h"
#include "proc.h"

// Program entry points (given by the linker)
extern unsigned char _binary_user_squares_elf_start[];
extern unsigned char _binary_user_pi_elf_start[];
extern unsigned char _binary_user_primecheck_elf_start[];
extern unsigned char _binary_user_counter_elf_start[];
extern unsigned char _binary_user_hello_elf_start[];

extern struct proc* current_process;

void load(void* executable, void* proc_address, struct proc* proc) {
  void* entry_point = load_elf(executable, proc_address);
  proc->state = RUNNABLE;

  // TODO: implement!
}

void run(struct proc* proc) {
  void (*f)(void) = (void (*)(void))proc->entry_point;
  (void)f;  // suppress unused variable warning

  current_process = proc;
  proc->state = RUNNING;
  f();  // TODO: what needs to change here?
}

void init() {
  // TODO: simpler example than what you need to do in the project
  // (the project has an `executables` array.
  struct proc* p1 = allocproc("HELLO");
  load((void*)_binary_user_hello_elf_start,
       (void*)PROC_START + p1->pid * PROC_SIZE, p1);
  struct proc* p2 = allocproc("COUNTER");
  load((void*)_binary_user_counter_elf_start,
       (void*)PROC_START + p2->pid * PROC_SIZE, p2);
  struct proc* p3 = allocproc("PI");
  load((void*)_binary_user_pi_elf_start,
       (void*)PROC_START + p3->pid * PROC_SIZE, p3);
  struct proc* p4 = allocproc("PRIMECHECK");
  load((void*)_binary_user_primecheck_elf_start,
       (void*)PROC_START + p4->pid * PROC_SIZE, p4);

  print_process_table();

  run(p1);

  while (true) {
  }
}
