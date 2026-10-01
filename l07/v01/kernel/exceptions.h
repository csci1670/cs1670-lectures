#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

int current_exception_level(void);
void exception_init(void* table_addr);

// Not really a function, but an exception vector table in the
// form of 128-byte slots of machine code.
void exception_vector_table();

#endif  // _EXCEPTIONS_H
