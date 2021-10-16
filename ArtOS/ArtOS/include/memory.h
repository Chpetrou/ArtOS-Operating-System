#ifndef __MEMORY_H__
#define __MEMORY_H__

/** Initialize the memory subsystem */
int memory_init(void);

/** Request physical page frames */
size_t get_pages(size_t npages);

/** Get a single page
 *
 * Convenience function: uses get_pages(1);
 */
static inline size_t get_page(void) { return get_pages(1); }

/** release physical page frames */
int put_pages(size_t phyaddr, size_t npages);

/** Put a single page
 *
 * Convenience function: uses put_pages(1);
 */
static inline int put_page(size_t phyaddr) { return put_pages(phyaddr, 1); }

/** Copy a physical page frame
 *
 * psrc physical address of source page frame
 * pdest physical address of source page frame
 * 
 * - 0 on success
 * - -1 on failure
 */
int copy_page(size_t pdest, size_t psrc);

#endif
