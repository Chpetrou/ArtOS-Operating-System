#ifndef __ARCH_IRQFLAGS_H__
#define __ARCH_IRQFLAGS_H__

#ifdef __cplusplus
extern "C" {
#endif

/** Disable IRQs
 *
 * This inline function just clears out the interrupt bit
 */
inline static void irq_disable(void) {
	asm volatile("cli" ::: "memory");
}

/** Disable IRQs (nested)
 *
 * Disable IRQs when unsure if IRQs were enabled at all.\n
 * This function together with irq_nested_enable can be used
 * in situations when interrupts shouldn't be activated if they
 * were not activated before calling this function.
 *
 * The set of flags which have been set until now
 */
inline static uint8_t irq_nested_disable(void) {
	size_t flags;
	asm volatile("pushf; cli; pop %0": "=r"(flags) : : "memory");
	if (flags & (1 << 9))
		return 1;	
	return 0;
}

/** Enable IRQs */
inline static void irq_enable(void) {
	asm volatile("sti" ::: "memory");
}

/** Enable IRQs (nested)
 *
 * If called after calling irq_nested_disable, this function will
 * not activate IRQs if they were not active before.
 *
 * flags Flags to set. Could be the old ones you got from irq_nested_disable.
 */
inline static void irq_nested_enable(uint8_t flags) {
	if (flags)
		irq_enable();
}

/** Determines, if the interrupt flags (IF) is ser
 *
 * 
 * - 1 interrupt flag is set
 * - 0 interrupt flag is cleared
 */ 
inline static uint8_t is_irq_enabled(void)
{
	size_t flags;
	asm volatile("pushf; pop %0": "=r"(flags) : : "memory");
	if (flags & (1 << 9))
		return 1;
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif
