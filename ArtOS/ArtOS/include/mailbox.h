#ifndef __MAILBOX_H__
#define __MAILBOX_H__

#include <string.h>
#include <mailbox_types.h>
#include <tasks.h>
#include <semaphore.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAILBOX(name, type) 	\
	inline static int mailbox_##name##_init(mailbox_##name##_t* m) { \
		if (BUILTIN_EXPECT(!m, 0)) \
			return -EINVAL; \
	\
		memset(m->buffer, 0x00, sizeof(type)*MAILBOX_SIZE); \
		m->wpos = m->rpos = 0; \
		sem_init(&m->mails, 0); \
		sem_init(&m->boxes, MAILBOX_SIZE); \
		lock_init(&m->rlock); \
		lock_init(&m->wlock); \
	\
		return 0; \
	}\
	\
	inline static int mailbox_##name##_destroy(mailbox_##name##_t* m) { \
		if (BUILTIN_EXPECT(!m, 0)) \
			return -EINVAL; \
	\
		sem_destroy(&m->mails); \
		sem_destroy(&m->boxes); \
		lock_destroy(&m->rlock); \
		lock_destroy(&m->wlock); \
	\
		return 0; \
	} \
	\
	inline static int mailbox_##name##_post(mailbox_##name##_t* m, type mail) { \
		if (BUILTIN_EXPECT(!m, 0)) \
			return -EINVAL; \
	\
		sem_wait(&m->boxes); \
		s_lock(&m->wlock); \
		m->buffer[m->wpos] = mail; \
		m->wpos = (m->wpos+1) % MAILBOX_SIZE; \
		s_unlock(&m->wlock); \
		sem_post(&m->mails); \
	\
		return 0; \
	} \
	\
	inline static int mailbox_##name##_trypost(mailbox_##name##_t* m, type mail) { \
		if (BUILTIN_EXPECT(!m, 0)) \
			return -EINVAL; \
	\
		if (sem_trywait(&m->boxes)) \
			return -EBUSY; \
		s_lock(&m->wlock); \
		m->buffer[m->wpos] = mail; \
		m->wpos = (m->wpos+1) % MAILBOX_SIZE; \
		s_unlock(&m->wlock); \
		sem_post(&m->mails); \
	\
		return 0; \
	} \
	\
	inline static int mailbox_##name##_fetch(mailbox_##name##_t* m, type* mail) { \
		int err; \
	\
		if (BUILTIN_EXPECT(!m || !mail, 0)) \
			return -EINVAL; \
	\
		err = sem_wait(&m->mails); \
		if (err) return err; \
		s_lock(&m->rlock); \
		*mail = m->buffer[m->rpos]; \
		m->rpos = (m->rpos+1) % MAILBOX_SIZE; \
		s_unlock(&m->rlock); \
		sem_post(&m->boxes); \
	\
		return 0; \
	} \
	\
	inline static int mailbox_##name##_tryfetch(mailbox_##name##_t* m, type* mail) { \
		if (BUILTIN_EXPECT(!m || !mail, 0)) \
			return -EINVAL; \
	\
		if (sem_trywait(&m->mails) != 0) \
			return -EINVAL; \
		s_lock(&m->rlock); \
		*mail = m->buffer[m->rpos]; \
		m->rpos = (m->rpos+1) % MAILBOX_SIZE; \
		s_unlock(&m->rlock); \
		sem_post(&m->boxes); \
	\
		return 0; \
	}\

MAILBOX(wait_msg, wait_msg_t)
MAILBOX(int32, int32_t)
MAILBOX(int16, int16_t)
MAILBOX(int8, int8_t)
MAILBOX(uint32, uint32_t)
MAILBOX(uint16, uint16_t)
MAILBOX(uint8, uint8_t)
MAILBOX(ptr, void*)

#ifdef __cplusplus
}
#endif

#endif
