#include <stdio.h>

//Some Configurations
atomic_int32_t kmsg_counter = ATOMIC_INIT(0);
unsigned char kmessages[KMSG_SIZE] __attribute__ ((section(".kmsg"))) = {[0 ... KMSG_SIZE-1] = 0x00};

