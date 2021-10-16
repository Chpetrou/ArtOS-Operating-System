#include <string.h>
#include <lowlevel/idt.h>

/* 
 * Declare an IDT of 256 entries. Although we will only use the
 * first 32 entries in this tutorial, the rest exists as a bit
 * of a trap. If any undefined IDT entry is hit, it normally
 * will cause an "Unhandled Interrupt" exception. Any descriptor
 * for which the 'presence' bit is cleared (0) will generate an
 * "Unhandled Interrupt" exception 
 */
static idt_entry_t idt[256] = {[0 ... 255] = {0, 0, 0, 0, 0}};
static idt_ptr_t idtp;

void configure_idt_entry(idt_entry_t *dest_entry, size_t base, unsigned short sel, unsigned char flags)
{
	/* The interrupt routine's base address */
	dest_entry->base_lo = (base & 0xFFFF);
	dest_entry->base_hi = (base >> 16) & 0xFFFF;

	/* The segment or 'selector' that this IDT entry will use
	 *  is set here, along with any access flags */
	dest_entry->sel = sel;
	dest_entry->always0 = 0;
	dest_entry->flags = flags;
}

/*
 * Use this function to set an entry in the IDT. Alot simpler
 * than twiddling with the GDT ;)
 */
void idt_set_gate(unsigned char num, size_t base, unsigned short sel, unsigned char flags)
{
	configure_idt_entry(&idt[num], base, sel, flags);
}

extern void isrsyscall(void);

/* Installs the IDT */
void idt_install(void)
{
	static int initialized = 0;

	if (!initialized) {
		initialized = 1;

		/* Sets the special IDT pointer up, just like in 'gdt.c' */
		idtp.limit = (sizeof(idt_entry_t) * 256) - 1;
		idtp.base = (size_t)&idt;

		/* Add any new ISRs to the IDT here using idt_set_gate */
		idt_set_gate(INT_SYSCALL, (size_t)isrsyscall, KERNEL_CODE_SELECTOR,
			IDT_FLAG_PRESENT|IDT_FLAG_RING3|IDT_FLAG_32BIT|IDT_FLAG_TRAPGATE);
	}

	/* Points the processor's internal register to the new IDT */
	asm volatile("lidt %0" : : "m" (idtp));
}
