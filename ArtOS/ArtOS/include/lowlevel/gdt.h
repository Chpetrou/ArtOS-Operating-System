#ifndef __ARCH_GDT_H__
#define __ARCH_GDT_H__

#include <sysdef.h>

#ifdef __cplusplus
extern "C" {
#endif

/// This segment is a data segment
#define GDT_FLAG_DATASEG	0x02
/// This segment is a code segment
#define GDT_FLAG_CODESEG	0x0a
#define GDT_FLAG_TSS		0x09
#define GDT_FLAG_TSS_BUSY	0x02

#define GDT_FLAG_SEGMENT	0x10
/// Privilege level: Ring 0 
#define GDT_FLAG_RING0		0x00
/// Privilege level: Ring 1
#define GDT_FLAG_RING1		0x20
/// Privilege level: Ring 2 
#define GDT_FLAG_RING2		0x40
/// Privilege level: Ring 3 
#define GDT_FLAG_RING3		0x60
/// Segment is present
#define GDT_FLAG_PRESENT        0x80
/// Segment was accessed
#define GDT_FLAG_ACCESSED       0x01
/** 
 * Granularity of segment limit 
 * - set: segment limit unit is 4 KB (page size)
 * - not set: unit is bytes
 */
#define GDT_FLAG_4K_GRAN	0x80
/**
 * Default operand size 
 * - set: 32 bit
 * - not set: 16 bit
 */
#define GDT_FLAG_16_BIT		0x00
#define GDT_FLAG_32_BIT		0x40
#define GDT_FLAG_64_BIT		0x20

/** Defines a GDT entry 
 *
 * A global descriptor table entry consists of:
 * - 32 bit base address (chunkwise embedded into this structure)
 * - 20 bit limit
 */
typedef struct {
	/// Lower 16 bits of limit range
	uint16_t limit_low;
	/// Lower 16 bits of base address
	uint16_t base_low;
	/// middle 8 bits of base address
	uint8_t base_middle;
	/// Access bits
	uint8_t access;
	/// Granularity bits
	uint8_t granularity;
	/// Higher 8 bits of base address
	uint8_t base_high;
} __attribute__ ((packed)) gdt_entry_t;

/** defines the GDT pointer structure
 *
 * This structure tells the address and size of the table.
 */
typedef struct {
	/// Size of the table in bytes (not the number of entries!)
	uint16_t limit;
	/// Address of the table
	size_t base;
} __attribute__ ((packed)) gdt_ptr_t;

#define GDT_ENTRIES	(5+1)

#if GDT_ENTRIES > 8192
#error Too many GDT entries!
#endif

/** Installs the global descriptor table
 *
 * The installation involves the following steps:
 * - set up the special GDT pointer 
 * - set up the entries in our GDT
 * - finally call gdt_flush() in our assembler file 
 *   in order to tell the processor where the new GDT is
 * - update the new segment registers 
 */
void gdt_install(void);

/** Set gate with chosen attributes
 */
void gdt_set_gate(int num, unsigned long base, unsigned long limit,
			  unsigned char access, unsigned char gran);

/** Configures and returns a GDT descriptor with chosen attributes
 *
 * Just feed this function with address, limit and the flags
 * you have seen in idt.h
 *
 * a preconfigured gdt descriptor
 */
void configure_gdt_entry(gdt_entry_t *dest_entry, unsigned long base, unsigned long limit,
		unsigned char access, unsigned char gran);

#ifdef __cplusplus
}
#endif

#endif
