#ifndef __TASKS_H__
#define __TASKS_H__

#include <sysdef.h>
#include <tasks_types.h>
#include <lowlevel/tasks.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Array of task structures (aka PCB)
 *
 * A task's id will be its position in this array.
 */
extern task_t task_table[MAX_TASKS];

extern lock_irq_t table_lock;

extern readyqueues_t readyqueues;

extern task_t* current_task;
extern const void boot_stack;
    
/** System call to terminate a user level process */
void NORETURN sys_exit(int);

/** Task switcher
 *
 * Timer-interrupted use of this function for task switching
 *
 * 
 * - 0 no context switch
 * - !0 address of the old stack pointer
 */
size_t** scheduler(void);

/** Initialize the multitasking subsystem
 *
 * This procedure sets the current task to the
 * current "task" (there are no tasks, yet) and that was it.
 *
 * 
 * - 0 on success
 * - -ENOMEM (-12) on failure
 */
int multitasking_init(void);

/** create a kernel-level task. 
 *
 * id The value behind this pointer will be set to the new task's id
 * ep Pointer to the entry function for the new task
 * args Arguments the task shall start with
 * prio Desired priority of the new kernel task
 *
 * 
 * - 0 on success
 * - -EINVAL (-22) on failure
 */
int create_kernel_task(tid_t* id, entry_point_t ep, void* args, uint8_t prio);

/** Create a user level task.
 *
 * id The value behind this pointer will be set to the new task's id
 * fname Filename of the executable to start the task with
 * argv Pointer to arguments array
 *
 * 
 * - 0 on success
 * - -EINVAL (-22) or -ENOMEM (-12)on failure
 */
int create_user_task(tid_t* id, const char* fame, char** argv);

/** Create a task with a specific entry point
 *
 * id Pointer to a tid_t struct were the id shall be set
 * ep Pointer to the function the task shall start with
 * arg Arguments list
 * prio Desired priority of the new task
 * core_id Start the new task on the core with this id
 *
 * 
 * - 0 on success
 * - -ENOMEM (-12) or -EINVAL (-22) on failure
 */
int create_task(tid_t* id, entry_point_t ep, void* arg, uint8_t prio);

/** Cleanup function for the task termination
 *
 * On termination, the task call this function to cleanup its address space.
 */
void finish_task_switch(void);

/** determine the highest priority of all tasks, which are ready
 *
 * 
 * - return highest priority
 * - if no task is ready, the function returns an invalid value (> MAX_PRIO)
 */
uint32_t get_highest_priority(void);

/** Call to rescheduling
 *
 * This is a purely assembled procedure for rescheduling
 */
void reschedule(void);

/** Wake up a blocked task
 *
 * The task's status will be changed to TASK_READY 
 *
 * 
 * - 0 on success
 * - -EINVAL (-22) on failure
 */
int wakeup_task(tid_t);

/** Block current task
 *
 * The current task's status will be changed to TASK_BLOCKED
 *
 * 
 * - 0 on success
 * - -EINVAL (-22) on failure
 */
int block_current_task(void);

/** Abort current task */
void NORETURN abort(void);

/** This function shall be called by leaving kernel-level tasks */
void NORETURN leave_kernel_task(void);

#ifdef __cplusplus
}
#endif

#endif
