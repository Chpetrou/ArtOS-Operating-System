#include <sysdef.h>
#include <stdio.h>
#include <tasks.h>
#include <errno.h>
#include <syscall.h>
#include <spinlock.h>
#include <terminal.h>

//System calls implementation

static lock_irq_t lwip_lock = LOCK_IRQ_INIT;

typedef struct {
    int sysnr;
    int fd;
    size_t len;
} __attribute__((packed)) sys_write_t;

static int sys_write(int __attribute__((unused))fd, const char* buf, size_t __attribute__((unused))len)
{
    if (BUILTIN_EXPECT(!buf, 0))
        return -EINVAL;
    
    sys_write_t sysargs = {__NR_write, fd, len};
    
    for(int i=0; i<len; i++)
        monPut(buf[i]);
    
    return len;
}

typedef struct {
    int sysnr;
    int fd;
    size_t len;
} __attribute__((packed)) sys_read_t;

static int sys_read(int fd, char *buf, size_t len) {
    sys_read_t sysargs = {__NR_read, fd, len};
    ssize_t j, ret = 1;
    char bf[5];
    
    if (fd == '\0') {
        kputs("error");
    }
    else {
        for (int i = 0; i < len; i++) {
            addToBuffer(buf, 5, getChar());
            ret++;
        }
    }
    
    return ret;
}

static ssize_t sys_sbrk(int incr)
{
	task_t* task = current_task;
	vma_t* heap = task->heap;
	ssize_t ret;

	s_lock(&task->vma_lock);

	if (BUILTIN_EXPECT(!heap, 0)) {
		kprintf("sys_sbrk: missing heap!\n");
		abort();
	}

	ret = heap->end;
	heap->end += incr;
	if (heap->end < heap->start)
		heap->end = heap->start;

	// allocation and mapping of new pages for the heap
	// is catched by the pagefault handler

	s_unlock(&task->vma_lock);

	return ret;
}

ssize_t syscall_handler(uint32_t sys_nr, ...)
{
	ssize_t ret = -EINVAL;
	va_list vl;

	va_start(vl, sys_nr);

	switch(sys_nr)
	{
	case __NR_exit:
		sys_exit(va_arg(vl, uint32_t));
		ret = 0;
		break;
	case __NR_write: {
		int fd = va_arg(vl, int);
		const char* buf = va_arg(vl, const char*);
		size_t len = va_arg(vl, size_t);
		ret = sys_write(fd, buf, len);
		break;
	}
    case __NR_read: {
        int fd = va_arg(vl, int);
        char* buf = va_arg(vl, char*);
        size_t len = va_arg(vl, size_t);
        ret = sys_read(fd, buf, len);
        break;
    }
	case __NR_open:
	case __NR_close:
		ret = 0;
		break;
	case __NR_sbrk: {
		int incr = va_arg(vl, int);

		ret = sys_sbrk(incr);
		break;
	}
	default:
		kprintf("invalid system call: %d\n", sys_nr);
		ret = -ENOSYS;
		break;
	};

	va_end(vl);

	return ret;
}
