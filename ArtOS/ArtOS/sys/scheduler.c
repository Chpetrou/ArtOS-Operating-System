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

size_t** scheduler(void)
{
    task_t* orig_task;
    uint32_t prio;
    
    orig_task = current_task;
    
    lock_irq(&readyqueues.lock);
    
    /* signalizes that this task could be reused */
    if (current_task->status == TASK_FINISHED) {
        current_task->status = TASK_INVALID;
        readyqueues.old_task = current_task;
    } else readyqueues.old_task = NULL; // reset old task
    
    prio = msb(readyqueues.prio_bitmap); // determines highest priority
    if (prio > MAX_PRIO) {
        if ((current_task->status == TASK_RUNNING) || (current_task->status == TASK_IDLE))
            goto get_task_out;
        current_task = readyqueues.idle;
    } else {
        // Does the current task have an higher priority? => no task switch
        if ((current_task->prio > prio) && (current_task->status == TASK_RUNNING))
            goto get_task_out;
        
        if (current_task->status == TASK_RUNNING) {
            current_task->status = TASK_READY;
            readyqueues.old_task = current_task;
        }
        
        current_task = readyqueues.queue[prio-1].first;
        if (BUILTIN_EXPECT(current_task->status == TASK_INVALID, 0)) {
            kprintf("Upps!!!!!!! Got invalid task %d, orig task %d\n", current_task->id, orig_task->id);
        }
        current_task->status = TASK_RUNNING;
        
        // remove new task from queue
        // by the way, priority 0 is only used by the idle task and doesn't need own queue
        readyqueues.queue[prio-1].first = current_task->next;
        if (!current_task->next) {
            readyqueues.queue[prio-1].last = NULL;
            readyqueues.prio_bitmap &= ~(1 << prio);
        }
        current_task->next = current_task->prev = NULL;
    }
    
get_task_out:
    unlock_irq(&readyqueues.lock);
    
    if (current_task != orig_task) {
        /* if the original task is using the FPU, we need to save the FPU context */
        if ((orig_task->flags & TASK_FPU_USED) && (orig_task->status == TASK_READY)) {
            save_fpu_state(&(orig_task->fpu));
            orig_task->flags &= ~TASK_FPU_USED;
        }
        
        //kprintf("schedule from %d to %d with prio %d\n", orig_task->id, current_task->id, (uint32_t)current_task->prio);
        
        return (size_t**) &(orig_task->last_stack_pointer);
    }
    
    return NULL;
}

//Reschedule task
void reschedule(void)
{
    size_t** stack;
    uint8_t flags;
    
    flags = irq_nested_disable();
    if ((stack = scheduler()))
        switch_context(stack);
    irq_nested_enable(flags);
}
