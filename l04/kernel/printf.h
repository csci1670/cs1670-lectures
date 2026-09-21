#ifndef _PRINTF_H
#define _PRINTF_H

#include "types.h"

int panic(const char *format, ...);
int printf(const char *format, ...);

int vprintf_kernel(const char *format, va_list args);

#endif
