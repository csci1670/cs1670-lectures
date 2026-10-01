#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "context.h"

extern struct proc* current_process;


[[noreturn]] void handle_yield(context_t* ctx);



#endif  // _SCHEDULER_H
