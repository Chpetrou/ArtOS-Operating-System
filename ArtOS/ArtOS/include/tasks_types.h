#ifndef __TASKS_TYPES_H__
#define __TASKS_TYPES_H__

#include <sysdef.h>
#include <spinlock_types.h>
#include <vma.h>
#include <lowlevel/tasks_types.h>
#include <lowlevel/atomic.h>

#ifdef __cplusplus
extern "C" {
#endif
    
#define TASK_INVALID	0
#define TASK_READY		1
#define TASK_RUNNING	2
#define TASK_BLOCKED	3
#define TASK_FINISHED	4
#define TASK_IDLE		5

#define TASK_DEFAULT_FLAGS	0
#define TASK_FPU_INIT		(1 << 0)
#define TASK_FPU_USED		(1 << 1)

#define MAX_PRIO	31
#define REALTIME_PRIO	31
#define HIGH_PRIO	16
#define NORMAL_PRIO	8
#define LOW_PRIO	1
#define IDLE_PRIO	0

typedef int (*entry_point_t)(void*);
    
/** Represents a the process control block */
typedef struct task {
	/// Task id = position in the task table
	tid_t			id __attribute__ ((aligned (CACHE_LINE)));
	/// Task status (INVALID, READY, RUNNING, ...)
	uint32_t		status;
	/// copy of the stack pointer before a context switch
	size_t*			last_stack_pointer;
	/// start address of the stack 
	void*			stack;
	/// Additional status flags. For instance, to signalize the using of the FPU
	uint8_t			flags;
	/// Task priority
	uint8_t			prio;
	/// Physical address of root page table
	size_t			page_map;
	/// Lock for page tables
	lock_irq_t	page_lock;
	/// lock for the VMA_list
	lock_t		vma_lock;
	/// list of VMAs
	vma_t*			vma_list;
	/// the userspace heap
	vma_t*			heap;
	/// usage in number of pages (including page map tables)
	atomic_int32_t	user_usage;
	/// next task in the queue
	struct task*	next;
	/// previous task in the queue
	struct task*	prev;
	/// FPU state
	union fpu_state	fpu;
} task_t;

typedef struct {
        task_t* first;
        task_t* last;
} task_list_t;

/** Represents a queue for all runable tasks */
typedef struct {
	/// idle task
	task_t*		idle __attribute__ ((aligned (CACHE_LINE)));
        /// previous task
	task_t*		old_task;
	/// total number of tasks in the queue
	uint32_t	nr_tasks;
	/// indicates the used priority queues
	uint32_t	prio_bitmap;
	/// a queue for each priority
	task_list_t	queue[MAX_PRIO];
	/// lock for this runqueue
	lock_irq_t lock;
} readyqueues_t;

#ifdef __cplusplus
}
#endif

#endif
