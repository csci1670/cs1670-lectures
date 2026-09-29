#ifndef __LIMITS_H
#define __LIMITS_H

#define NPROC 16            // Maximum number of processes
#define PROC_START 0xA0000  // Start address of the first process
#define PROC_SIZE 0x10000   // Size of a process
#define PROCNAME_MAXLEN 16  // Maximum length of process name

#define NUM_EXECUTABLES 3  // Number of executables linked in the kernel

#endif  // __LIMITS_H
