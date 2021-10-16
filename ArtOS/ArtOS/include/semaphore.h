#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <string.h>
#include <tasks.h>
#include <semaphore_types.h>
#include <spinlock.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Semaphore initialization
 *
 * Always init semaphores before use!
 *
 * s Pointer to semaphore structure to initialize
 * v Resource count
 *
 * 
 * - 0 on success
 * - -EINVAL on invalid argument
 */
inline static int sem_init(sem_t* s, unsigned int v) {
	unsigned int i;

	if (BUILTIN_EXPECT(!s, 0))
		return -EINVAL;

	s->value = v;
	s->pos = 0;
	for(i=0; i<MAX_TASKS; i++)
		s->queue[i] = MAX_TASKS;
	lock_irq_init(&s->lock);

	return 0;
}

/** Destroy semaphore
 * 
 * - 0 on success
 * - -EINVAL on invalid argument
 */
inline static int sem_destroy(sem_t* s) {
	if (BUILTIN_EXPECT(!s, 0))
		return -EINVAL;

	lock_irq_destroy(&s->lock);

	return 0;
}

/** Nonblocking trywait for sempahore
 *
 * Will return immediately if not available
 *
 * 
 * - 0 on success (You got the semaphore)
 * - -EINVAL on invalid argument
 * - -ECANCELED on failure (You still have to wait)
 */
inline static int sem_trywait(sem_t* s) {
	int ret = -ECANCELED;

	if (BUILTIN_EXPECT(!s, 0))
		return -EINVAL;

	lock_irq(&s->lock);
	if (s->value > 0) {
		s->value--;
		ret = 0;
	}
	unlock_irq(&s->lock);

	return ret;
}

/** Blocking wait for semaphore
 *
 * s Address of the according sem_t structure
 * 
 * - 0 on success
 * - -EINVAL on invalid argument
 * - -ETIME on timer expired
 */
inline static int sem_wait(sem_t* s) {
	if (BUILTIN_EXPECT(!s, 0))
		return -EINVAL;

next_try1:
	lock_irq(&s->lock);
	if (s->value > 0) {
		s->value--;
		unlock_irq(&s->lock);
	} else {
		s->queue[s->pos] = current_task->id;
		s->pos = (s->pos + 1) % MAX_TASKS;
		block_current_task();
		unlock_irq(&s->lock);
		reschedule();
		goto next_try1;
	}

	return 0;
}

/** Give back resource 
 * 
 * - 0 on success
 * - -EINVAL on invalid argument
 */
inline static int sem_post(sem_t* s) {
	if (BUILTIN_EXPECT(!s, 0))
		return -EINVAL;

	lock_irq(&s->lock);
	if (s->value > 0) {
		s->value++;
		unlock_irq(&s->lock);
	} else {
		unsigned int k, i;

		s->value++;
		i = s->pos;
		for(k=0; k<MAX_TASKS; k++) {
			if (s->queue[i] < MAX_TASKS) {
				wakeup_task(s->queue[i]);
				s->queue[i] = MAX_TASKS;
				break;
			}
			i = (i + 1) % MAX_TASKS;
		}
		unlock_irq(&s->lock);
	}

	return 0;
}

#ifdef __cplusplus
}
#endif

#endif
