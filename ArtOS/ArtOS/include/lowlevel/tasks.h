#ifndef __ASM_TASKS_H__
#define __ASM_TASKS_H__

#include <sysdef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Switch to current task
 *
 * stack Pointer to the old stack pointer
 */
void switch_context(size_t** stack);

/** Setup a default frame for a new task
 *
 * task Pointer to the task structure
 * ep The entry point for code execution
 * arg Arguments list pointer for the task's stack
 * 
 * - 0 on success
 * - -EINVAL (-22) on failure
 */
int create_default_frame(task_t* task, entry_point_t ep, void* arg);

/** Register a task's TSS at GDT
 *
 * 
 * - 0 on success
 */
static inline int register_task(void)
{
	uint16_t sel = 5 << 3;

	asm volatile ("ltr %%ax" : : "a"(sel));

	return 0;
}

/** Jump to user code
 *
 * This function runs the user code after stopping it just as if
 * it was a return from a procedure.
 *
 * 0 in any case
 */
static inline int jump_to_user_code(size_t ep, size_t stack)
{
	// Create a pseudo interrupt on the stack and return to user function
	size_t ds = 0x23, cs = 0x1b;

	asm volatile ("mov %0, %%ds; mov %0, %%es" :: "r"(ds));
	asm volatile ("push %0; push %1; pushf; push %2; push %3" :: "r"(ds), "r"(stack), "r"(cs), "r"(ep));
	asm volatile ("iret");

	return 0;
}

#ifdef __cplusplus
}
#endif

#endif
