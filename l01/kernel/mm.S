// memzero function implemented in ARM64 assembly.
// Overwrites memory contents with zeroes for a configurable length.
//
// memzero(void* ptr, size_t len)
// - ptr is passed in register x0
// - len is passed in register x1

    .globl memzero
memzero:
    str    xzr, [x0], #8   // write 8 zero bytes at [x0], then advance x0 by 8
    subs   x1, x1, #8      // len -= 8, and update the condition flags
    b.gt   memzero         // if len is still > 0, loop
    ret                    // otherwise return (jump to x30)
