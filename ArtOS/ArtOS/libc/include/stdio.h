#ifndef _STDIO_H
#define _STDIO_H 1
 
#include "sys/cdefs.h"
#include <stdarg.h>
#include <sysdef.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <spinlock.h>
#include <fs.h>
#include <lowlevel/atomic.h>
#include <lowlevel/processor.h>
#include <lowlevel/multiboot.h>

#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

#define NO_EARLY_PRINT        0x00
#define VGA_EARLY_PRINT        0x01

/* Predefined file handles. */
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
    
int realBufferSize;
    
extern uint32_t early_print;
extern lock_irq_t olock;
extern atomic_int32_t kmsg_counter;
extern unsigned char kmessages[KMSG_SIZE] __attribute__ ((section(".kmsg")));

struct vfs_node;
int kmsg_init(struct vfs_node* node, const char *name);
    
extern int sprintf(char *buf, const char *fmt, ...);
extern int kprintf(const char *fmt, ...);
int kputchar(int);
int kputs(const char*);
int kscanf(const char *fmt, ...);
int readStr(char* var_addr);
int readInt(int var_addr);
 
#ifdef __cplusplus
}
#endif
 
#endif
