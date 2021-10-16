#ifndef __ARCH_SYSCALL_H__
#define __ARCH_SYSCALL_H__

#include <sysdef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _STR(token)		#token
#define _SYSCALLSTR(x)	"int $" _STR(x) " "

/** the syscall function which issues an interrupt to the kernel
 *
 * It's supposed to be used by the macros defined in this file as the could would read
 * cleaner then.
 *
 * nr System call number
 * arg0 Argument 0
 * arg1 Argument 1
 * arg2 Argument 2
 * arg3 Argument 3
 * arg4 Argument 4
 * The return value of the system call
 */
inline static long
syscall(int nr, unsigned long arg0, unsigned long arg1, unsigned long arg2,
	unsigned long arg3, unsigned long arg4)
{
	long res;

	asm volatile (_SYSCALLSTR(INT_SYSCALL)
			: "=a" (res)
			: "0" (nr), "b" (arg0), "c" (arg1), "d" (arg2), "S" (arg3), "D" (arg4)
			: "memory", "cc");

	return res;
}

/// System call macro with one single argument; the syscall number
#define SYSCALL0(NR) \
	syscall(NR, 0, 0, 0, 0, 0)
#define SYSCALL1(NR, ARG0) \
	syscall(NR, (unsigned long)ARG0, 0, 0, 0, 0)
#define SYSCALL2(NR, ARG0, ARG1) \
	syscall(NR, (unsigned long)ARG0, (unsigned long)ARG1, 0, 0, 0)
#define SYSCALL3(NR, ARG0, ARG1, ARG2) \
	syscall(NR, (unsigned long)ARG0, (unsigned long)ARG1, (unsigned long)ARG2, 0, 0)
#define SYSCALL4(NR, ARG0, ARG1, ARG2, ARG3) \
	syscall(NR, (unsigned long)ARG0, (unsigned long)ARG1, (unsigned long)ARG2, (unsigned long) ARG3, 0)
#define SYSCALL5(NR, ARG0, ARG1, ARG2, ARG3, ARG4) \
	syscall(NR, (unsigned long)ARG0, (unsigned long)ARG1, (unsigned long)ARG2, (unsigned long) ARG3, (unsigned long) ARG4)

#ifdef __cplusplus
}
#endif

#endif
