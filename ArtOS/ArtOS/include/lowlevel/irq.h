#ifndef __ARCH_IRQ_H__
#define __ARCH_IRQ_H__

#include <sysdef.h>
#include <tasks_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Pointer-type to IRQ-handling functions
 *
 * Whenever you write a IRQ-handling function it has to match this signature.
 */
typedef void (*irq_handler_t)(struct registers *);

/** Install a custom IRQ handler for a given IRQ 
 *
 * irq The desired irq
 * handler The handler to install
 */
int irq_install_handler(unsigned int irq, irq_handler_t handler);

/** Clear the handler for a given IRQ 
 *
 * irq The handler's IRQ
 */
int irq_uninstall_handler(unsigned int irq);

/** Procedure to initialize IRQ
 *
 * This procedure is just a small collection of calls:
 * - idt_install();
 * - isrs_install();
 * - irq_install();
 *
 * Just returns 0 in any case
 */
int irq_init(void);

#ifdef __cplusplus
}
#endif

#endif
