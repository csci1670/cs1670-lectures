#ifndef _TYPES_H
#define _TYPES_H

// Unsigned integer types
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long uint64;

// Signed integer types
typedef signed char int8;
typedef short int16;
typedef int int32;
typedef long int64;

// Size types, size_t and ssize_t (signed size)
typedef uint64 size_t;
typedef int64 ssize_t;

// General pointer type
typedef uint64 uintptr_t;

// Alias stdarg functions to their compiler-builtin equivalents.
// This is necessary to handle functions with a variable number
// of arguments (chiefly, `printf`).
typedef __builtin_va_list va_list;
#define va_start(val, last) __builtin_va_start(val, last)
#define va_arg(val, type) __builtin_va_arg(val, type)
#define va_end(val) __builtin_va_end(val)

#endif  // _TYPES_H
