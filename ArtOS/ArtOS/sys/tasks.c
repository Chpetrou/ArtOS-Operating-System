#include <sysdef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tasks.h>
#include <tasks_types.h>
#include <spinlock.h>
#include <errno.h>
#include <syscall.h>
#include <memory.h>

task_t task_table[MAX_TASKS] = { \
    [0]                 = {0, TASK_IDLE, NULL, NULL, TASK_DEFAULT_FLAGS, 0, 0, LOCK_IRQ_INIT, LOCK_INIT, NULL, NULL, ATOMIC_INIT(0), NULL, NULL}, \
    [1 ... MAX_TASKS-1] = {0, TASK_INVALID, NULL, NULL, TASK_DEFAULT_FLAGS, 0, 0, LOCK_IRQ_INIT, LOCK_INIT, NULL, NULL,ATOMIC_INIT(0), NULL, NULL}};

lock_irq_t table_lock = LOCK_IRQ_INIT;

readyqueues_t readyqueues = {task_table+0, NULL, 0, 0, {[0 ... MAX_PRIO-2] = {NULL, NULL}}, LOCK_IRQ_INIT};

task_t* current_task = task_table+0;

/** helper function for the assembly code to determine the current task
 * Pointer to the task_t structure of current task
 */
task_t* get_current_task(void)
{
	return current_task;
}

uint32_t get_highest_priority(void)
{
	return msb(readyqueues.prio_bitmap);
}

int multitasking_init(void)
{
	if (BUILTIN_EXPECT(task_table[0].status != TASK_IDLE, 0)) {
		kputs("Task 0 is not an idle task\n");
		return -ENOMEM;
	}

	task_table[0].prio = IDLE_PRIO;
	task_table[0].stack = (void*) &boot_stack;
	task_table[0].page_map = read_cr3();

	// register idle task
	register_task();

	return 0;
}

void finish_task_switch(void)
{
	task_t* old;
	uint8_t prio;

	lock_irq(&readyqueues.lock);

	if ((old = readyqueues.old_task) != NULL) {
		if (old->status == TASK_INVALID) {
			old->stack = NULL;
			old->last_stack_pointer = NULL;
			readyqueues.old_task = NULL;
		} else {
			prio = old->prio;
			if (!readyqueues.queue[prio-1].first) {
				old->next = old->prev = NULL;
				readyqueues.queue[prio-1].first = readyqueues.queue[prio-1].last = old;
			} else {
				old->next = NULL;
				old->prev = readyqueues.queue[prio-1].last;
				readyqueues.queue[prio-1].last->next = old;
				readyqueues.queue[prio-1].last = old;
			}
			readyqueues.old_task = NULL;
			readyqueues.prio_bitmap |= (1 << prio);
		}
	}

	unlock_irq(&readyqueues.lock);

	if (current_task->heap)
		kfree(current_task->heap);
}

/** A procedure to be called by
 * procedures which are called by exiting tasks. */
static void NORETURN do_exit(int arg)
{
	task_t* curr_task = current_task;

//    kprintf("Terminate task: %d, return value %d\n", curr_task->id, arg);

	page_map_drop();

	// decrease the number of active tasks
	lock_irq(&readyqueues.lock);
	readyqueues.nr_tasks--;
	unlock_irq(&readyqueues.lock);

	curr_task->status = TASK_FINISHED;
	reschedule();

	kprintf("Kernel panic: scheduler found no valid task\n");
	while(1) {
		HALT;
	}
}

/** A procedure to be called by kernel tasks */
void NORETURN leave_kernel_task(void) {
	int result;

	result = 0; //get_return_value();
	do_exit(result);
}

/** To be called by the systemcall to exit tasks */
void NORETURN sys_exit(int arg) {
	do_exit(arg);
}

/** Aborting a task is like exiting it with result -1 */
void NORETURN abort(void) {
	do_exit(-1);
}

int create_task(tid_t* id, entry_point_t ep, void* arg, uint8_t prio)
{
	int ret = -ENOMEM;
	uint32_t i;

	if (BUILTIN_EXPECT(!ep, 0))
		return -EINVAL;
	if (BUILTIN_EXPECT(prio == IDLE_PRIO, 0))
		return -EINVAL;
	if (BUILTIN_EXPECT(prio > MAX_PRIO, 0))
		return -EINVAL;

	lock_irq(&table_lock);

	for(i=0; i<MAX_TASKS; i++) {
		if (task_table[i].status == TASK_INVALID) {
			task_table[i].id = i;
			task_table[i].status = TASK_READY;
			task_table[i].last_stack_pointer = NULL;
			task_table[i].stack = create_stack(i);
			task_table[i].prio = prio;
			lock_init(&task_table[i].vma_lock);
			task_table[i].vma_list = NULL;
			task_table[i].heap = NULL;

			lock_irq_init(&task_table[i].page_lock);
			atomic_int32_set(&task_table[i].user_usage, 0);

			/* Allocated new PGD or PML4 and copy page table */
			task_table[i].page_map = get_pages(1);
			if (BUILTIN_EXPECT(!task_table[i].page_map, 0))
				goto out;

			/* Copy page tables & user frames of current task to new one */
			page_map_copy(&task_table[i]);

			if (id)
				*id = i;

			ret = create_default_frame(task_table+i, ep, arg);

			// add task in the readyqueues
			lock_irq(&readyqueues.lock);
			readyqueues.prio_bitmap |= (1 << prio);
			readyqueues.nr_tasks++;
			if (!readyqueues.queue[prio-1].first) {
				task_table[i].next = task_table[i].prev = NULL;
				readyqueues.queue[prio-1].first = task_table+i;
				readyqueues.queue[prio-1].last = task_table+i;
			} else {
				task_table[i].prev = readyqueues.queue[prio-1].last;
				task_table[i].next = NULL;
				readyqueues.queue[prio-1].last->next = task_table+i;
				readyqueues.queue[prio-1].last = task_table+i;
			}
			unlock_irq(&readyqueues.lock);
			break;
		}
	}

out:
	unlock_irq(&table_lock);

	return ret;
}

int create_kernel_task(tid_t* id, entry_point_t ep, void* args, uint8_t prio)
{
	if (prio > MAX_PRIO)
		prio = NORMAL_PRIO;

	return create_task(id, ep, args, prio);
}

/** Wakeup a blocked task
 * id The task's tid_t structure
 * 
 * - 0 on success
 * - -EINVAL (-22) on failure
 */
int wakeup_task(tid_t id)
{
	task_t* task;
	uint32_t prio;
	int ret = -EINVAL;
	uint8_t flags;

	flags = irq_nested_disable();

	task = task_table + id;
	prio = task->prio;

	if (task->status == TASK_BLOCKED) {
		task->status = TASK_READY;
		ret = 0;

		lock_irq(&readyqueues.lock);
		// increase the number of ready tasks
		readyqueues.nr_tasks++;

		// add task to the runqueue
		if (!readyqueues.queue[prio-1].last) {
			readyqueues.queue[prio-1].last = readyqueues.queue[prio-1].first = task;
			task->next = task->prev = NULL;
			readyqueues.prio_bitmap |= (1 << prio);
		} else {
			task->prev = readyqueues.queue[prio-1].last;
			task->next = NULL;
			readyqueues.queue[prio-1].last->next = task;
			readyqueues.queue[prio-1].last = task;
		}
		unlock_irq(&readyqueues.lock);
	}

	irq_nested_enable(flags);

	return ret;
}

/** Block current task
 *
 * The current task's status will be changed to TASK_BLOCKED
 *
 * 
 * - 0 on success
 * - -EINVAL (-22) on failure
 */
int block_current_task(void)
{
	tid_t id;
	uint32_t prio;
	int ret = -EINVAL;
	uint8_t flags;

	flags = irq_nested_disable();

	id = current_task->id;
	prio = current_task->prio;

	if (task_table[id].status == TASK_RUNNING) {
		task_table[id].status = TASK_BLOCKED;
		ret = 0;

		lock_irq(&readyqueues.lock);
		// reduce the number of ready tasks
		readyqueues.nr_tasks--;

		// remove task from queue
		if (task_table[id].prev)
			task_table[id].prev->next = task_table[id].next;
		if (task_table[id].next)
			task_table[id].next->prev = task_table[id].prev;
		if (readyqueues.queue[prio-1].first == task_table+id)
			readyqueues.queue[prio-1].first = task_table[id].next;
		if (readyqueues.queue[prio-1].last == task_table+id) {
			readyqueues.queue[prio-1].last = task_table[id].prev;
			if (!readyqueues.queue[prio-1].last)
				readyqueues.queue[prio-1].last = readyqueues.queue[prio-1].first;
		}

		// No valid task in queue => update prio_bitmap
		if (!readyqueues.queue[prio-1].first)
			readyqueues.prio_bitmap &= ~(1 << prio);
		unlock_irq(&readyqueues.lock);
	}

	irq_nested_enable(flags);

	return ret;
}
