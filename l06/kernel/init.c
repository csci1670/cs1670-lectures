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
extern struct proc processes[];

void load(void* executable, void* proc_address, struct proc* proc);
void run(struct proc* proc);

unsigned char* executables[] = {
  _binary_user_hello_elf_start,
  _binary_user_counter_elf_start,
  _binary_user_pi_elf_start,
  _binary_user_primecheck_elf_start,
  nullptr,
};

char* executable_names[] = {
  "hello",
  "counter",
  "pi",
  "primecheck",
  nullptr,
};


void init() {
  // Create a process for each program in the exeuctables array
  for(int i = 0; executables[i] != nullptr; i++) {
    struct proc* p = allocproc(executable_names[i]);
    load((void*)executables[i],
         (void*)PROC_START + p->pid * PROC_SIZE, p);

  }
  print_process_table();

  run(&processes[0]);

  while (true) {
  }
}

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
