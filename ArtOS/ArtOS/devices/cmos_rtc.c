#include <lowlevel/io.h>
#include <rtc.h>
#include <stdio.h>
 
unsigned char seconds;
unsigned char minutes;
unsigned char hours;
unsigned char days;
unsigned char months;
unsigned int years;

int century_reg = 0x32; // century register

int progress_flag_updates()
{
      outportb(cmos_address, 0x0A);
      return (inportb(cmos_data) & 0x80);
}
 
unsigned char rtc_register(int reg)
{
      outportb(cmos_address, reg);
      return inportb(cmos_data);
}
 
void read_rtc()
{
      unsigned char century;
      unsigned char last_second;
      unsigned char last_minute;
      unsigned char last_hour;
      unsigned char last_day;
      unsigned char last_month;
      unsigned char last_year;
      unsigned char last_century;
      unsigned char registerB;
      century=0;
      // Note: This uses the "read registers until you get the same values twice in a row" technique
      //       to avoid getting dodgy/inconsistent values due to RTC updates
 
      while (progress_flag_updates());                // Make sure an update isn't in progress
      seconds = rtc_register(0x00);
      minutes = rtc_register(0x02);
      hours = rtc_register(0x04);
      days = rtc_register(0x07);
      months = rtc_register(0x08);
      years = rtc_register(0x09);

      if(century_reg != 0)
      {
            century = rtc_register(century_reg);
      }
 
      do {
            last_second = seconds;
            last_minute = minutes;
            last_hour = hours;
            last_day = days;
            last_month = months;
            last_year = years;
            last_century = century;
 
            while (progress_flag_updates());           // Make sure an update isn't in progress
            seconds = rtc_register(0x00);
            minutes = rtc_register(0x02);
            hours = rtc_register(0x04);
            days = rtc_register(0x07);
            months = rtc_register(0x08);
            years = rtc_register(0x09);
            if(century_reg != 0) {
                  century = rtc_register(century_reg);
            }
      } while( (last_second != seconds) || (last_minute != minutes) || (last_hour != hours) ||
               (last_day != days) || (last_month != months) || (last_year != years) ||
               (last_century != century) );
 
      registerB = rtc_register(0x0B);
 
      // Convert BCD to binary values if necessary
 
      if (!(registerB & 0x04)) {
            seconds = (seconds & 0x0F) + ((seconds / 16) * 10);
            minutes = (minutes & 0x0F) + ((minutes / 16) * 10);
            hours = ( (hours & 0x0F) + (((hours & 0x70) / 16) * 10) ) | (hours & 0x80);
            days = (days & 0x0F) + ((days / 16) * 10);
            months = (months & 0x0F) + ((months / 16) * 10);
            years = (years & 0x0F) + ((years / 16) * 10);
            if(century_reg != 0) {
                  century = (century & 0x0F) + ((century / 16) * 10);
            }
      }
 
      // Convert 12 hour clock to 24 hour clock if necessary
 
      if (!(registerB & 0x02) && (hours & 0x80)) {
            hours = ((hours & 0x7F) + 12) % 24;
      }
 
      // Calculate the full (4-digit) year
 
      if(century_reg != 0) {
            years += century * 100;
      } else {
            years += (CURRENT_YEAR / 100) * 100;
            if(years < CURRENT_YEAR) years += 100;
      }
      if(years<2017)
      {
            kprintf("ArtOS was made in 2017, are you the time traveller?", years);
      }
}
