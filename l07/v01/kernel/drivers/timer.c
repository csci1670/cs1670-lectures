#include "drivers/timer.h"

#include "debug.h"
#include "interrupts.h"
#include "printf.h"
#include "types.h"

#define TIMER_FREQ_HZ  10 // Timer interrupts at 10 Hz (10 interrupts/second)


uint64 period;                 // Timer period in hardware ticks


void timer_set(uint64 interval_ticks);


void timer_init(void) {
  uint64 hw_tick_rate = 0;  // In ticks per second
  // Read hardware timer's frequency (number of ticks per second)
  // from timer register.
  // This varies across QEMU and real hardware (62.5 MHz vs. 19.2 MHz)
  __asm__ volatile("mrs %[hw_tick_rate], CNTFRQ_EL0"
                   : [hw_tick_rate] "=r"(hw_tick_rate)
                   :);

  // Mask out top 32 bits (which are reserved)
  hw_tick_rate &= 0xFFFFFFFF;

  // Compute the timer period (number of ticks between interrupts)
  period = hw_tick_rate / TIMER_FREQ_HZ;

  // Set first timer interrupt
  timer_set(period);

  // Turn on timer (sets the ENABLE bit of the timer control register)
  __asm__ volatile("msr CNTP_CTL_EL0, %0" ::"r"((uint64)1));

}

void timer_set(uint64 interval_ticks) {
  __asm__ volatile("msr CNTP_TVAL_EL0, %0" ::"r"(interval_ticks));
}

void timer_reset(void) {
  // Restart the timer
  // For this particular timer implementation, we need to restart the timer
  // on each interrupt
  timer_set(period);
}
