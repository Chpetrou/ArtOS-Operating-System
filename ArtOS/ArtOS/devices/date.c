#include <rtc.h>
#include <stdio.h>
#include <time.h>
#include <sysdef.h>
#include <terminal.h>
int date_time()
{
    read_rtc();
    //Time in 3 different languages
    switch (language) {
        case GREEK:
            kprintf("I ora kai imerominia twra einai:\n"
                    "%02d:%02d:%02d\n"
                    "%02d/%02d/%04d\n",hours+1,minutes,seconds,days,months,years);
            break;
        case SPANISH:
            kprintf("La  hora/fecha actual es:\n"
                    "%02d:%02d:%02d\n"
                    "%02d/%02d/%04d\n",hours+1,minutes,seconds,days,months,years);
            break;
        case ENGLISH:
            kprintf("Current time/date is:\n"
                    "%02d:%02d:%02d\n"
                    "%02d/%02d/%04d\n",hours+1,minutes,seconds,days,months,years);
            break;
        default:
            break;
    }
    return 0;
}
