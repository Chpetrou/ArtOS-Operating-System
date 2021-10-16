#ifndef __STDDEF_H__
#define __STDDEF_H__

#include <config.h>
#include <lowlevel/stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NULL         ((void*) 0)

/// represents a task identifier
typedef unsigned int tid_t;

struct task;
/// pointer to the current (running) task
extern struct task* current_task;

#ifdef __cplusplus
}
#endif

#endif
