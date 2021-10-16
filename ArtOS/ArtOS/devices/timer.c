//#include <stdio.h>
//#include <string.h>
//#include <tasks.h>
#include <time.h>
//#include <errno.h>
//#include <lowlevel/irq.h>
//#include <lowlevel/irqflags.h>
//#include <lowlevel/io.h>

/* 
 * This will keep track of how many ticks the system
 * has been running for 
 */
static volatile uint64_t timer_ticks = 0;
int seconds;

//Get the clock ticks
uint64_t get_clock_tick(void)
{
	return timer_ticks;
}

//Wait until some ticks
void waitTicks(uint64_t ticks) {
    uint64_t finalTicks = timer_ticks + ticks;
    while (timer_ticks < finalTicks);
}

//Wait until some seconds
void waitForTime(uint64_t secondsWait){
    seconds = 0;
    do {
        if (timer_ticks % TIMER_FREQ == 0) seconds++;
    } while (seconds < secondsWait);
    
}

/* 
 * Handles the timer. In this case, it's very simple: We
 * increment the 'timer_ticks' variable every time the
 * timer fires. 
 */
static void timer_handler(struct registers *s)
{
	/* Increment our 'tick counter' */
	timer_ticks++;

	/*
	 * Every TIMER_FREQ clocks (approximately 1 second), we will
	 * display a message on the screen
	 */
	//if (timer_ticks % TIMER_FREQ == 0) {
	//	vga_puts("One second has passed\n");
	//}
}

#define LATCH(f)	((CLOCK_TICK_RATE + f/2) / f)

/* 
 * Sets up the system clock by installing the timer handler
 * into IRQ123
 */
int timer_init(void)
{
	/* 
	 * Installs 'timer_handler' for the PIC and APIC timer,
	 * only one handler will be later used.
	 */
	irq_install_handler(32, timer_handler);
	irq_install_handler(123, timer_handler);

	/*
	 * Port 0x43 is for initializing the PIT:
	 *
	 * 0x34 means the following:
	 * 0b...     (step-by-step binary representation)
	 * ...  00  - channel 0
	 * ...  11  - write two values to counter register:
	 *            first low-, then high-byte
	 * ... 010  - mode number 2: "rate generator" / frequency divider
	 * ...   0  - binary counter (the alternative is BCD)
	 */
	outportb(0x43, 0x34);

	WAIT();

	/* Port 0x40 is for the counter register of channel 0 */

	outportb(0x40, LATCH(TIMER_FREQ) & 0xFF);   /* low byte  */

	WAIT();

	outportb(0x40, LATCH(TIMER_FREQ) >> 8);     /* high byte */

	return 0;
}
