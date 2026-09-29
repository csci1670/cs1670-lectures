#ifndef _CONSTANT_H
#define _CONSTANT_H

// Size of a saved context_t (see proc.h):
// 31 general-purpose registers
// + SP_EL0
// + ELR_EL1
// + SPSR_EL1
// = 34 * 8 bytes.
// (16-byte aligned, as required for stack pointer).
#define S_FRAME_SIZE 272

#endif
