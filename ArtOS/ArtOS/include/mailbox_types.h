#ifndef __MAILBOX_TYPES_H__
#define __MAILBOX_TYPES_H__

#include <semaphore_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Wait message structure
 *
 * This message struct keeps a recipient task id and the message itself */
typedef struct {
	/// The task id of the task which is waiting for this message
	tid_t	id;
	/// The message payload
	int32_t	result;
} wait_msg_t;

#define MAILBOX_TYPES(name, type) 	\
	typedef struct mailbox_##name { \
		type buffer[MAILBOX_SIZE]; \
		int wpos, rpos; \
		sem_t mails; \
		sem_t boxes; \
		lock_t rlock, wlock; \
	} mailbox_##name##_t;

MAILBOX_TYPES(wait_msg, wait_msg_t)
MAILBOX_TYPES(int32, int32_t)
MAILBOX_TYPES(int16, int16_t)
MAILBOX_TYPES(int8, int8_t)
MAILBOX_TYPES(uint32, uint32_t)
MAILBOX_TYPES(uint16, uint16_t)
MAILBOX_TYPES(uint8, uint8_t)
MAILBOX_TYPES(ptr, void*)

#ifdef __cplusplus
}
#endif

#endif
