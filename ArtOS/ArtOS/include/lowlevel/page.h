#include <sysdef.h>
#include <stdlib.h>

#ifndef __PAGE_H__
#define __PAGE_H__

/// Page offset bits
#define PAGE_BITS		12
/// The size of a single page in bytes
#define PAGE_SIZE		( 1L << PAGE_BITS)
/// Mask the page address without page map flags and XD flag
#define PAGE_MASK		(-1L << PAGE_BITS)

/// Total operand width in bits
#define BITS			32
/// Physical address width (we dont support PAE)
#define PHYS_BITS		BITS
/// Linear/virtual address width
#define VIRT_BITS		BITS
/// Page map bits
#define PAGE_MAP_BITS	10
/// Number of page map indirections
#define PAGE_LEVELS		2

/// Make address canonical
#define CANONICAL(addr)		(addr) // only for 32 bit paging

/// The number of entries in a page map table
#define PAGE_MAP_ENTRIES	       (1L << PAGE_MAP_BITS)

/// Align to next page
#define PAGE_FLOOR(addr)        (((addr) + PAGE_SIZE - 1) & PAGE_MASK)
/// Align to page
#define PAGE_CEIL(addr)         ( (addr)                  & PAGE_MASK)

/// Page is present
#define PG_PRESENT		(1 << 0)
/// Page is read- and writable
#define PG_RW			(1 << 1)
/// Page is addressable from userspace
#define PG_USER			(1 << 2)
/// Page write through is activated
#define PG_PWT			(1 << 3)
/// Page cache is disabled
#define PG_PCD			(1 << 4)
/// Page was recently accessed (set by CPU)
#define PG_ACCESSED		(1 << 5)
/// Page is dirty due to recent write-access (set by CPU)
#define PG_DIRTY		(1 << 6)
/// Huge page: 4MB (or 2MB, 1GB)
#define PG_PSE			(1 << 7)
/// Page attribute table
#define PG_PAT			PG_PSE
/// Global TLB entry (Pentium Pro and later)
#define PG_GLOBAL		(1 << 8)
/// This table is a self-reference and should skipped by page_map_copy()
#define PG_SELF			(1 << 9)

/** Converts a virtual address to a physical
 *
 * A non mapped virtual address causes a pagefault!
 *
 * addr Virtual address to convert
 * physical address
 */
size_t virt_to_phys(size_t vir);

/** Initialize paging subsystem
 *
 * This function uses the existing bootstrap page tables (boot_{pgd, pgt})
 * to map required regions (video memory, kernel, etc..).
 * Before calling page_init(), the bootstrap tables contain a simple identity
 * paging. Which is replaced by more specific mappings.
 */
int page_init(void);

/** Map a continuous region of pages
 *
 * viraddr Desired virtual address
 * phyaddr Physical address to map from
 * npages The region's size in number of pages
 * bits Further page flags
 * 
 */
int page_map(size_t viraddr, size_t phyaddr, size_t npages, size_t bits);

/** Unmap a continuous region of pages
 *
 * viraddr The virtual start address
 * npages The range's size in pages
 * 
 */
int page_unmap(size_t viraddr, size_t npages);

/** Change the page permission in the page tables of the current task
 *
 * Applies given flags noted in the 'flags' parameter to
 * the range denoted by virtual start and end addresses.
 *
 * start Range's virtual start address
 * end Range's virtual end address
 * flags flags to apply
 *
 * 
 * - 0 on success
 * - -EINVAL (-22) on failure.
 */
int page_set_flags(size_t viraddr, uint32_t npages, int flags);

/** Copy a whole page map tree
 *
 * dest Physical address of new page map
 * @retval 0 Success. Everything went fine.
 * @retval <0 Error. Something went wrong.
 */
int page_map_copy(struct task *dest);

/** Free a whole page map tree */
int page_map_drop(void);

#endif
