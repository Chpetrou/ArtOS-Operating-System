#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <sysdef.h>
#include <lowlevel/syscall.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __NR_exit 		0
#define __NR_write		1
#define __NR_open		2
#define __NR_close		3
#define __NR_read		4
#define __NR_lseek		6
#define __NR_unlink		7
#define __NR_getpid		8
#define __NR_kill		9
#define __NR_fstat		10
#define __NR_sbrk		11
#define __NR_fork		12
#define __NR_wait		13
#define __NR_execve		14
#define __NR_times		15
#define __NR_accept		16
#define __NR_bind		17
#define __NR_closesocket	18
#define __NR_connect		19
#define __NR_listen		20
#define __NR_recv		21
#define __NR_send		22
#define __NR_socket		23
#define __NR_getsockopt		24
#define __NR_setsockopt		25
#define __NR_gethostbyname	26
#define __NR_sendto		27
#define __NR_recvfrom		28
#define __NR_select		29
#define __NR_stat		30
#define __NR_dup		31
#define __NR_dup2		32

#ifdef __cplusplus
}
#endif

#endif
