#ifndef __TIME_H__
#define __TIME_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <string.h>
#include <tasks.h>
#include <errno.h>
#include <lowlevel/irq.h>
#include <lowlevel/irqflags.h>
#include <lowlevel/io.h>

/** Initialize Timer interrupts 
 *
 * This procedure installs IRQ handlers for timer interrupts
 */
int timer_init(void);

void waitForTime(uint64_t secondsWait);
void waitTicks(uint64_t ticks);
    
/** Returns the current number of ticks.
 * Current number of ticks
 */
uint64_t get_clock_tick(void);
    
#define WAIT() do { uint64_t start = rdtsc(); \
while(rdtsc() - start < 1000000) ; \
} while (0)

#ifdef __cplusplus
}
#endif

#endif
