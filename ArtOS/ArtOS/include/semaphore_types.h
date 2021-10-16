#ifndef __SEMAPHORE_TYPES_H__
#define __SEMAPHORE_TYPES_H__

#include <spinlock_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Semaphore structure */
typedef struct {
	/// Resource available count
	unsigned int value;
	/// Queue of waiting tasks
	tid_t queue[MAX_TASKS];
	/// Position in queue
	unsigned int pos;
	/// Access lock
	lock_irq_t lock;
} sem_t;

/// Macro for initialization of semaphore
#define SEM_INIT(v) {v, {[0 ... MAX_TASKS-1] = MAX_TASKS}, 0, LOCK_IRQ_INIT}

#ifdef __cplusplus
}
#endif

#endif
