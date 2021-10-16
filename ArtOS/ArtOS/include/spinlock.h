#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include <sysdef.h>
#include <spinlock_types.h>
#include <tasks_types.h>
#include <errno.h>
#include <lowlevel/atomic.h>
#include <lowlevel/processor.h>
#include <lowlevel/irqflags.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialization of a spinlock
 *
 * Initialize each spinlock before use!
 *
 * s Pointer to the spinlock structure to initialize.
 * 
 * - 0 on success
 * - -EINVAL (-22) on failure
 */
inline static int lock_init(lock_t* s) {
	if (BUILTIN_EXPECT(!s, 0))
		return -EINVAL;

	atomic_int32_set(&s->queue, 0);
	atomic_int32_set(&s->dequeue, 1);
	s->owner = MAX_TASKS;
	s->counter = 0;

	return 0;
}

/** Destroy spinlock after use
 * 
 * - 0 on success
 * - -EINVAL (-22) on failure
 */
inline static int lock_destroy(lock_t* s) {
	if (BUILTIN_EXPECT(!s, 0))
		return -EINVAL;

	s->owner = MAX_TASKS;
	s->counter = 0;

	return 0;
}

/** Lock spinlock at entry of critical section 
 * 
 * - 0 on success
 * - -EINVAL (-22) on failure
 */
inline static int s_lock(lock_t* s) {
	int32_t ticket;

	if (BUILTIN_EXPECT(!s, 0))
		return -EINVAL;

	if (s->owner == current_task->id) {
		s->counter++;
		return 0;
	}

	ticket = atomic_int32_add(&s->queue, 1);
	while(atomic_int32_read(&s->dequeue) != ticket) {
		PAUSE;
	}
	s->owner = current_task->id;
	s->counter = 1;

	return 0;
}

/** Unlock spinlock on exit of critical section 
 * 
 * - 0 on success
 * - -EINVAL (-22) on failure
 */
inline static int s_unlock(lock_t* s) {
	if (BUILTIN_EXPECT(!s, 0))
		return -EINVAL;

	s->counter--;
	if (!s->counter) {
		s->owner = MAX_TASKS;
		atomic_int32_inc(&s->dequeue);
	}

	return 0;
}

/** Initialization of a irqsave spinlock
 *
 * Initialize each irqsave spinlock before use!
 *
 * 
 * - 0 on success
 * - -EINVAL (-22) on failure
 */
inline static int lock_irq_init(lock_irq_t* s) {
	if (BUILTIN_EXPECT(!s, 0))
		return -EINVAL;

	atomic_int32_set(&s->queue, 0);
	atomic_int32_set(&s->dequeue, 1);
	s->flags = 0;
	s->counter = 0;

	return 0;
}

/** Destroy irqsave spinlock after use
 * 
 * - 0 on success
 * - -EINVAL (-22) on failure
 */
inline static int lock_irq_destroy(lock_irq_t* s) {
	if (BUILTIN_EXPECT(!s, 0))
		return -EINVAL;

	s->flags = 0;
	s->counter = 0;

	return 0;
}

/** Unlock an irqsave spinlock on exit of critical section 
 * 
 * - 0 on success
 * - -EINVAL (-22) on failure
 */
inline static int lock_irq(lock_irq_t* s) {
	int32_t ticket;
	uint8_t flags;

	if (BUILTIN_EXPECT(!s, 0))
		return -EINVAL;

	flags = irq_nested_disable();
	if (s->counter == 1) {
		s->counter++;
		return 0;
	}

	ticket = atomic_int32_add(&s->queue, 1);
	while (atomic_int32_read(&s->dequeue) != ticket) {
		PAUSE;
	}

	s->flags = flags;
	s->counter = 1;

	return 0;
}

/** Unlock irqsave spinlock on exit of critical section and re-enable interrupts
 * 
 * - 0 on success
 * - -EINVAL (-22) on failure
 */
inline static int unlock_irq(lock_irq_t* s) {
	uint8_t flags;

	if (BUILTIN_EXPECT(!s, 0))
		return -EINVAL;

	s->counter--;
	if (!s->counter) {
		flags = s->flags;
		s->flags = 0;
                atomic_int32_inc(&s->dequeue);
		irq_nested_enable(flags);
	}

	return 0;
}

#ifdef __cplusplus
}
#endif

#endif
