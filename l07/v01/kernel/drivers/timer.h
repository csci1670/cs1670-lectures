#ifndef _DRIVERS_TIMER_H
#define _DRIVERS_TIMER_H

#include "memlayout.h"

// BCM 2837 data sheet, §12.1 (p. 172)
#define TIMER_BASE (PERIPHERALS_BASE + 0x0) // TODO
// . . .



// Initialize the timer
// Takes in a pointer to a "callback" function to call on each interrupt
void timer_init();


// Housekeeping:  reset timer for next interrupt
void timer_reset(void);


#endif  // _DRIVERS_TIMER_H
