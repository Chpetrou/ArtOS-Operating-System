#ifndef __SPINLOCK_TYPES_H__
#define __SPINLOCK_TYPES_H__

#include <sysdef.h>
#include <lowlevel/atomic.h>

#ifdef __cplusplus
extern "C" {
#endif


/** Spinlock structure */
typedef struct slock {
	/// Internal queue
	atomic_int32_t queue;
	/// Internal dequeue 
	atomic_int32_t dequeue;
	/// Owner of this spinlock structure
	tid_t owner;
	/// Internal counter var
	uint32_t counter;
} lock_t;

typedef struct lock_irqs {
	/// Internal queue
	atomic_int32_t queue;
	/// Internal dequeue
	atomic_int32_t dequeue;
	/// Internal counter var
	uint32_t counter;
	/// Interrupt flag
	uint8_t flags;
} lock_irq_t;

/// Macro for spinlock initialization
#define LOCK_INIT { ATOMIC_INIT(0), ATOMIC_INIT(1), MAX_TASKS, 0}
/// Macro for irqsave spinlock initialization
#define LOCK_IRQ_INIT { ATOMIC_INIT(0), ATOMIC_INIT(1), 0, 0}

#ifdef __cplusplus
}
#endif

#endif
