#ifndef __ARCH_IDT_H__
#define __ARCH_IDT_H__

#include <sysdef.h>

/// This bit shall be set to 0 if the IDT slot is empty
#define IDT_FLAG_PRESENT 	0x80
/// Interrupt can be called from within RING0
#define IDT_FLAG_RING0		0x00
/// Interrupt can be called from within RING1 and lower
#define IDT_FLAG_RING1		0x20
/// Interrupt can be called from within RING2 and lower
#define IDT_FLAG_RING2		0x40
/// Interrupt can be called from within RING3 and lower
#define IDT_FLAG_RING3		0x60
/// Size of gate is 16 bit
#define IDT_FLAG_16BIT		0x00
/// Size of gate is 32 bit
#define IDT_FLAG_32BIT		0x08
/// The entry describes an interrupt gate
#define IDT_FLAG_INTTRAP	0x06
/// The entry describes a trap gate
#define IDT_FLAG_TRAPGATE	0x07
/// The entry describes a task gate
#define IDT_FLAG_TASKGATE	0x05

/* 
 * This is not IDT-flag related. It's the segment selectors for kernel code and data.
 */
#define KERNEL_CODE_SELECTOR	0x08
#define KERNEL_DATA_SELECTOR	0x10

#ifdef __cplusplus
extern "C" {
#endif

/** Defines an IDT entry
 *
 * This structure defines interrupt descriptor table entries.\n
 * They consist of the handling function's base address, some flags 
 * and a segment selector.
 */
typedef struct {
	/// Handler function's lower 16 address bits
	uint16_t base_lo;
	/// Handler function's segment selector.
	uint16_t sel;
	/// These bits are reserved by Intel
	uint8_t always0;
	/// These 8 bits contain flags. Exact use depends on the type of interrupt gate.
	uint8_t flags;
	/// Higher 16 bits of handler function's base address
	uint16_t base_hi;
} __attribute__ ((packed)) idt_entry_t;

/** Defines the idt pointer structure.
 *
 * This structure keeps information about 
 * base address and size of the interrupt descriptor table.
 */
typedef struct {
	/// Size of the IDT in bytes (not the number of entries!)
	uint16_t limit;
	/// Base address of the IDT
	size_t base;
} __attribute__ ((packed)) idt_ptr_t;

/** Installs IDT
 *
 * The installation involves the following steps:
 * - Set up the IDT pointer
 * - Set up int 0x80 for syscalls
 * - process idt_load()
 */
void idt_install(void);

/** Set an entry in the IDT
 *
 * num index in the IDT
 * base base-address of the handler function being installed
 * sel Segment the IDT will use
 * flags Flags this entry will have
 */
void idt_set_gate(unsigned char num, size_t base, unsigned short sel,
		  unsigned char flags);

/** Configures and returns a IDT entry with chosen attributes
 *
 * Just feed this function with base, selector and the flags
 * you have seen in idt.h
 *
 * a preconfigured idt descriptor
 */
void configure_idt_entry(idt_entry_t *dest_entry, size_t base, 
		unsigned short sel, unsigned char flags);

#ifdef __cplusplus
}
#endif

#endif
