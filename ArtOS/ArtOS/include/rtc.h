#ifndef RTC_H
#define RTC_H

#include <time_date.h>

#define CURRENT_YEAR    2018 // Change this each year!

extern unsigned char seconds;
extern unsigned char minutes;
extern unsigned char hours;
extern unsigned char days;
extern unsigned char months;
extern unsigned int years;

enum
{
	cmos_address = 0x70,
	cmos_data    = 0x71
};

int progress_flag_updates();
unsigned char rtc_register(int reg);
void read_rtc();
int date_time();

#endif
