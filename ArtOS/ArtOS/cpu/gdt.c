#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <tasks.h>
#include <errno.h>
#include <processor.h>
#include <time.h>
#include <lowlevel/gdt.h>
#include <lowlevel/tss.h>
#include <lowlevel/page.h>

gdt_ptr_t				gp;
static tss_t			task_state_segment __attribute__ ((aligned (PAGE_SIZE)));
// currently, our kernel has full access to the ioports
static gdt_entry_t		gdt[GDT_ENTRIES] = {[0 ... GDT_ENTRIES-1] = {0, 0, 0, 0, 0, 0}};

/* 
 * This is defined in entry.asm. We use this to properly reload
 * the new segment registers
 */
extern void gdt_flush(void);

extern const void boot_stack;

void set_kernel_stack(void)
{
	task_t* curr_task = current_task;

	task_state_segment.esp0 = (size_t) curr_task->stack + KERNEL_STACK_SIZE - 16; // => stack is 16byte aligned
}

size_t get_kernel_stack(void)
{
	task_t* curr_task = current_task;

	return (size_t) curr_task->stack + KERNEL_STACK_SIZE - 16;
}

/* Setup a descriptor in the Global Descriptor Table */
void gdt_set_gate(int num, unsigned long base, unsigned long limit,
			  unsigned char access, unsigned char gran)
{
	configure_gdt_entry(&gdt[num], base, limit, access, gran);
}

void configure_gdt_entry(gdt_entry_t *dest_entry, unsigned long base, unsigned long limit,
		unsigned char access, unsigned char gran)
{
	/* Setup the descriptor base address */
	dest_entry->base_low = (base & 0xFFFF);
	dest_entry->base_middle = (base >> 16) & 0xFF;
	dest_entry->base_high = (base >> 24) & 0xFF;

	/* Setup the descriptor limits */
	dest_entry->limit_low = (limit & 0xFFFF);
	dest_entry->granularity = ((limit >> 16) & 0x0F);

	/* Finally, set up the granularity and access flags */
	dest_entry->granularity |= (gran & 0xF0);
	dest_entry->access = access;
}

/* 
 * This will setup the special GDT
 * pointer, set up the entries in our GDT, and then
 * finally call gdt_flush() in our assembler file in order
 * to tell the processor where the new GDT is and update the
 * new segment registers 
 */
void gdt_install(void)
{
	unsigned long gran_ds, gran_cs, limit;
	int num = 0;

	memset(&task_state_segment, 0x00, sizeof(tss_t));

	gran_cs = gran_ds = GDT_FLAG_32_BIT | GDT_FLAG_4K_GRAN;
	limit = 0xFFFFFFFF;

	/* Setup the GDT pointer and limit */
	gp.limit = (sizeof(gdt_entry_t) * GDT_ENTRIES) - 1;
	gp.base = (size_t) &gdt;

	/* Our NULL descriptor */
	gdt_set_gate(num++, 0, 0, 0, 0);

	/* 
	 * The second entry is our Code Segment. The base address
	 * is 0, the limit is 4 GByte, it uses 4KByte granularity,
	 * and is a Code Segment descriptor.
	 */
	gdt_set_gate(num++, 0, limit,
		GDT_FLAG_RING0 | GDT_FLAG_SEGMENT | GDT_FLAG_CODESEG | GDT_FLAG_PRESENT, gran_cs);

	/* 
	 * The third entry is our Data Segment. It's EXACTLY the
	 * same as our code segment, but the descriptor type in
	 * this entry's access byte says it's a Data Segment 
	 */
	gdt_set_gate(num++, 0, limit,
		GDT_FLAG_RING0 | GDT_FLAG_SEGMENT | GDT_FLAG_DATASEG | GDT_FLAG_PRESENT, gran_ds);

	/*
	 * Create code segment for 32bit user-space applications (ring 3)
	 */
	gdt_set_gate(num++, 0, 0xFFFFFFFF,
		GDT_FLAG_RING3 | GDT_FLAG_SEGMENT | GDT_FLAG_CODESEG | GDT_FLAG_PRESENT, GDT_FLAG_32_BIT | GDT_FLAG_4K_GRAN);

	/*
	 * Create data segment for user-space applications (ring 3)
	 */
	gdt_set_gate(num++, 0, limit,
		GDT_FLAG_RING3 | GDT_FLAG_SEGMENT | GDT_FLAG_DATASEG | GDT_FLAG_PRESENT, gran_ds);

	/* set default values */
	task_state_segment.eflags = 0x1202;
	task_state_segment.ss0 = 0x10;			// data segment
	task_state_segment.esp0 = (size_t) &boot_stack - 0x10;
	task_state_segment.cs = 0x0b;
	task_state_segment.ss = task_state_segment.ds = task_state_segment.es = task_state_segment.fs = task_state_segment.gs = 0x13;
	gdt_set_gate(num++, (unsigned long) (&task_state_segment), sizeof(tss_t)-1,
			GDT_FLAG_PRESENT | GDT_FLAG_TSS | GDT_FLAG_RING0, gran_ds);

	/* Flush out the old GDT and install the new changes! */
	gdt_flush();
}
