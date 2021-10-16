#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ARTOS_VERSION		"0.2"
#define MAX_TASKS		16
#define MAX_FILENAME		128
#define TIMER_FREQ		100 /* in HZ */
#define CLOCK_TICK_RATE		1193182 /* 8254 chip's internal oscillator frequency */
#define VIDEO_MEM_ADDR		0xB8000 /* the video memory address */
#define CACHE_LINE		64
#define KERNEL_STACK_SIZE	(8<<10)   /*  8 KiB */
#define DEFAULT_STACK_SIZE	(16*1024) /* 16 KiB */
#define BITMAP_SIZE		(128<<5) /* for 128 MiB of RAM */
#define KMSG_SIZE		(8*1024)
#define INT_SYSCALL		0x80
#define MAILBOX_SIZE	32

#define BYTE_ORDER		LITTLE_ENDIAN

#define CONFIG_PCI

#define BUILTIN_EXPECT(exp, b) 	__builtin_expect((exp), (b))
//#define BUILTIN_EXPECT(exp, b)	(exp)
#define NORETURN 		__attribute__((noreturn))
#define STDCALL 		__attribute__((stdcall))

#ifdef __cplusplus
}
#endif

#endif
