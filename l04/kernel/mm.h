#ifndef _MM_H
#define _MM_H

#ifndef __ASSEMBLER__

// Function to zero out a block of memory, implemented in
// assembly in `mm.S`.
void memzero(void* ptr, unsigned long len);

#endif

#endif  // _MM_H
