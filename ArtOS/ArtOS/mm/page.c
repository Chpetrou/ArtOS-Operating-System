#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <string.h>
#include <spinlock.h>

#include <lowlevel/irq.h>
#include <lowlevel/page.h>
#include <lowlevel/multiboot.h>

/* Note that linker symbols are not variables, they have no memory
 * allocated for maintaining a value, rather their address is their value. */
extern const void kernel_start;
//extern const void kernel_end;

/// This page is reserved for copying
#define PAGE_TMP		(PAGE_FLOOR((size_t) &kernel_start) - PAGE_SIZE)

/** Lock for kernel space page tables */
static lock_t kslock = LOCK_INIT;

/** This PGD table is initialized in entry.asm */
extern size_t* boot_map;

/** A self-reference enables direct access to all page tables */
static size_t * const self[PAGE_LEVELS] = {
	(size_t *) 0xFFC00000,
	(size_t *) 0xFFFFF000
};

/** An other self-reference for page_map_copy() */
static size_t * const other[PAGE_LEVELS] = {
	(size_t *) 0xFF800000,
	(size_t *) 0xFFFFE000
};

size_t virt_to_phys(size_t addr)
{
	size_t vpn   = addr >> PAGE_BITS;	// virtual page number
	size_t entry = self[0][vpn];		// page table entry
	size_t off   = addr  & ~PAGE_MASK;	// offset within page
	size_t phy   = entry &  PAGE_MASK;	// physical page frame number

	return phy | off;
}

int page_set_flags(size_t viraddr, uint32_t npages, int flags)
{
	return -EINVAL;
}

int page_map(size_t viraddr, size_t phyaddr, size_t npages, size_t bits)
{
	int lvl, ret = -ENOMEM;
	long vpn = viraddr >> PAGE_BITS;
	long first[PAGE_LEVELS], last[PAGE_LEVELS];

	/* Calculate index boundaries for page map traversal */
	for (lvl=0; lvl<PAGE_LEVELS; lvl++) {
		first[lvl] = (vpn         ) >> (lvl * PAGE_MAP_BITS);
		last[lvl]  = (vpn+npages-1) >> (lvl * PAGE_MAP_BITS);
	}

	if (bits & PG_USER)
		lock_irq(&current_task->page_lock);
	else
		s_lock(&kslock);

	/* Start iterating through the entries
	 * beginning at the root table (PGD or PML4) */
	for (lvl=PAGE_LEVELS-1; lvl>=0; lvl--) {
		for (vpn=first[lvl]; vpn<=last[lvl]; vpn++) {
			if (lvl) { /* PML4, PDPT, PGD */
				if (!(self[lvl][vpn] & PG_PRESENT)) {
					/* There's no table available which covers the region.
					 * Therefore we need to create a new empty table. */
					size_t phyaddr = get_pages(1);
					if (BUILTIN_EXPECT(!phyaddr, 0))
						goto out;
					
					if (bits & PG_USER)
						atomic_int32_inc(&current_task->user_usage);

					/* Reference the new table within its parent */
					self[lvl][vpn] = phyaddr | bits | PG_PRESENT | PG_USER | PG_RW;

					/* Fill new table with zeros */
					memset(&self[lvl-1][vpn<<PAGE_MAP_BITS], 0, PAGE_SIZE);
				}
			}
			else { /* PGT */
				if (self[lvl][vpn] & PG_PRESENT)
					/* There's already a page mapped at this address.
					 * We have to flush a single TLB entry. */
					tlb_flush_one_page(vpn << PAGE_BITS);

				self[lvl][vpn] = phyaddr | bits | PG_PRESENT;
				phyaddr += PAGE_SIZE;
			}
		}
	}

	ret = 0;
out:
	if (bits & PG_USER)
		unlock_irq(&current_task->page_lock);
	else
		s_unlock(&kslock);

	return ret;
}

/** Tables are freed by page_map_drop() */
int page_unmap(size_t viraddr, size_t npages)
{
	/* We aquire both locks for kernel and task tables
	 * as we dont know to which the region belongs. */
	lock_irq(&current_task->page_lock);
	s_lock(&kslock);

	/* Start iterating through the entries.
	 * Only the PGT entries are removed. Tables remain allocated. */
	size_t vpn, start = viraddr>>PAGE_BITS;
	for (vpn=start; vpn<start+npages; vpn++)
		self[0][vpn] = 0;

	unlock_irq(&current_task->page_lock);
	s_unlock(&kslock);

	/* This can't fail because we don't make checks here */
	return 0;
}

int page_map_drop(void)
{
	void traverse(int lvl, long vpn) {
		long stop;
		for (stop=vpn+PAGE_MAP_ENTRIES; vpn<stop; vpn++) {
			if ((self[lvl][vpn] & PG_PRESENT) && (self[lvl][vpn] & PG_USER)) {
				/* Post-order traversal */
				if (lvl)
					traverse(lvl-1, vpn<<PAGE_MAP_BITS);

				put_pages(self[lvl][vpn] & PAGE_MASK, 1);
				atomic_int32_dec(&current_task->user_usage);
			}
		}
	}

	lock_irq(&current_task->page_lock);

	traverse(PAGE_LEVELS-1, 0);

	unlock_irq(&current_task->page_lock);

	/* This can't fail because we don't make checks here */
	return 0;
}

int page_map_copy(task_t *dest)
{
	int traverse(int lvl, long vpn) {
		long stop;
		for (stop=vpn+PAGE_MAP_ENTRIES; vpn<stop; vpn++) {
			if (self[lvl][vpn] & PG_PRESENT) {
				if (self[lvl][vpn] & PG_USER) {
					size_t phyaddr = get_pages(1);
					if (BUILTIN_EXPECT(!phyaddr, 0))
						return -ENOMEM;

					atomic_int32_inc(&dest->user_usage);

					other[lvl][vpn] = phyaddr | (self[lvl][vpn] & ~PAGE_MASK);
					if (lvl) /* PML4, PDPT, PGD */
						traverse(lvl-1, vpn<<PAGE_MAP_BITS); /* Pre-order traversal */
					else { /* PGT */
						page_map(PAGE_TMP, phyaddr, 1, PG_RW);
						memcpy((void*) PAGE_TMP, (void*) (vpn<<PAGE_BITS), PAGE_SIZE);
					}
				}
				else if (self[lvl][vpn] & PG_SELF)
					other[lvl][vpn] = 0;
				else
					other[lvl][vpn] = self[lvl][vpn];
			}
			else
				other[lvl][vpn] = 0;
		}
		return 0;
	}

	lock_irq(&current_task->page_lock);
	self[PAGE_LEVELS-1][PAGE_MAP_ENTRIES-2] = dest->page_map | PG_PRESENT | PG_SELF | PG_RW;

	int ret = traverse(PAGE_LEVELS-1, 0);

	other[PAGE_LEVELS-1][PAGE_MAP_ENTRIES-1] = dest->page_map | PG_PRESENT | PG_SELF | PG_RW;
	self [PAGE_LEVELS-1][PAGE_MAP_ENTRIES-2] = 0;
	unlock_irq(&current_task->page_lock);

	/* Flush TLB entries of 'other' self-reference */
	flush_tlb();

	return ret;
}

void page_fault_handler(struct registers *s)
{
	size_t viraddr = read_cr2();
	task_t* task = current_task;

	// on demand userspace heap mapping
	if ((task->heap) && (viraddr >= task->heap->start) && (viraddr < task->heap->end)) {
		viraddr &= PAGE_MASK;

		size_t phyaddr = get_page();
		if (BUILTIN_EXPECT(!phyaddr, 0)) {
			kprintf("out of memory: task = %d\n", task->id);
			goto default_handler;
		}

		int ret = page_map(viraddr, phyaddr, 1, PG_USER|PG_RW);
		if (BUILTIN_EXPECT(ret, 0)) {
			kprintf("map_region: could not map %#lx to %#lx, task = %d\n", phyaddr, viraddr, task->id);
			put_page(phyaddr);

			goto default_handler;
		}

		memset((void*) viraddr, 0x00, PAGE_SIZE); // fill with zeros

		return;
	}

default_handler:
	kprintf("Page Fault Exception (%d) at cs:ip = %#x:%#lx, task = %d, addr = %#lx, error = %#x [ %s %s %s %s %s ]\n",
		s->int_no, s->cs, s->eip, current_task->id, viraddr, s->error,
		(s->error & 0x4) ? "user" : "supervisor",
		(s->error & 0x10) ? "instruction" : "data",
		(s->error & 0x2) ? "write" : ((s->error & 0x10) ? "fetch" : "read"),
		(s->error & 0x1) ? "protection" : "not present",
		(s->error & 0x8) ? "reserved bit" : "\b");

	while(1) HALT;
}

int page_init(void)
{
	size_t addr, npages;
	int i;

	/* Replace default pagefault handler */
	irq_uninstall_handler(14);
	irq_install_handler(14, page_fault_handler);

	/* Map multiboot information and modules */
	if (mb_info) {
		// already mapped => entry.asm
		//addr = (size_t) mb_info & PAGE_MASK;
		//npages = PAGE_FLOOR(sizeof(*mb_info)) >> PAGE_BITS;
		//page_map(addr, addr, npages, PG_GLOBAL);

		if (mb_info->flags & MULTIBOOT_INFO_MODS) {
			addr = mb_info->mods_addr;
			npages = PAGE_FLOOR(mb_info->mods_count*sizeof(multiboot_module_t)) >> PAGE_BITS;
			page_map(addr, addr, npages, PG_GLOBAL);
//            kprintf("Map module info at 0x%lx\n", addr);

			multiboot_module_t* mmodule = (multiboot_module_t*) ((size_t) mb_info->mods_addr);
			for(i=0; i<mb_info->mods_count; i++) {
				addr = mmodule[i].mod_start;
				npages = PAGE_FLOOR(mmodule[i].mod_end - mmodule[i].mod_start) >> PAGE_BITS;
				page_map(addr, addr, npages, PG_GLOBAL);
//                kprintf("Map modules at 0x%lx\n", addr);
			}
		}
	}

	return 0;
}
