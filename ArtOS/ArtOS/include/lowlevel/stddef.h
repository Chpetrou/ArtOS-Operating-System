#ifndef __ARCH_STDDEF_H__
#define __ARCH_STDDEF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
    
#if __SIZEOF_POINTER__ == 4

#define CONFIG_X86_32
#define KERNEL_SPACE	(1UL << 30) /*  1 GiB */

/// This type is used to represent the size of an object.
typedef unsigned long size_t;
/// Pointer differences
typedef long ptrdiff_t;
/// It is similar to size_t, but must be a signed type.
typedef long ssize_t;
/// The type represents an offset and is similar to size_t, but must be a signed type.
typedef long off_t;
//#elif __SIZEOF_POINTER__ == 8
//
//#define CONFIG_X86_64
//#define KERNEL_SPACE (1ULL << 30)
//
//// A popular type for addresses
//typedef unsigned long long size_t;
///// Pointer differences
//typedef long long ptrdiff_t;
//#ifdef __KERNEL__
//typedef long long ssize_t;
//typedef long long off_t;
//#endif
//#else
//#error unsupported architecture
#endif

/// This defines what the stack looks like after the task context is saved.
struct registers {
	/// ds register
	uint32_t ds;
	/// es register
	uint32_t es;
	/// EDI register
	uint32_t edi;
	/// ESI register
	uint32_t esi;
	/// EBP register
	uint32_t ebp;
	/// ESP register
	uint32_t esp;
	/// EBX register 
	uint32_t ebx;
	/// EDX register
	uint32_t edx;
	/// ECX register
	uint32_t ecx;
	/// EAX register
	uint32_t eax;		/* pushed by 'pusha' */

	/// Interrupt number
	uint32_t int_no;

	// pushed by the processor automatically
	uint32_t error;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
	uint32_t useresp;
	uint32_t ss;
};

#ifdef __cplusplus
}
#endif

#endif
