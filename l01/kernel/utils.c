#include "utils.h"

#include "printf.h"

unsigned int delay_cycles(uint64 ctr) {
  while (ctr--) {
    __asm__ volatile("nop");
  }
  return ctr;
}

void* memmove(void* dest, const void* src, uint64 n) {
  // We use 8-bit (1 byte) pointers to copy byte-by-byte.
  uint8* d = dest;
  const uint8* s = src;

  if (d == s || n == 0) {
    return dest;
  }

  if (d < s) {  // Copy forwards if destination before source
    for (uint64 i = 0; i < n; i++) {
      d[i] = s[i];
    }
  } else {
    // Source before destination, copy backwards to handle overlap
    for (uint64 i = n; i != 0; i--) {
      d[i - 1] = s[i - 1];
    }
  }

  return dest;
}

void* memcpy(void* dest, const void* src, uint64 n) {
  return memmove(dest, src, n);
}

void* memset(void* src, int c, uint64 n) {
  int* s = src;

  for (uint64 i = 0; i < n; ++i) {
    s[i] = c;
  }

  return src;
}
