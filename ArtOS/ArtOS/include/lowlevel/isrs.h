#ifndef __ARCH_ISRS_H__
#define __ARCH_ISRS_H__

#include <sysdef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** ISR installer procedure
 *
 * This procedure sets the first 32 entries in the IDT 
 * to the first 32 ISRs to call one general fault handler 
 * which will do some dispatching and exception message logging.\n
 * The access flags are set to 0x8E (PRESENT, privilege: RING0, size: 32bit gate, type: interrupt gate). 
 */
void isrs_install(void);

#ifdef __cplusplus
}
#endif

#endif
