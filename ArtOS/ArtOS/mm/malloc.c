#include <stdio.h>
#include <malloc.h>
#include <spinlock.h>
#include <memory.h>
#include <lowlevel/page.h>

/// A linked list for each binary size exponent
static pnode_t* pnode_lists[PNODE_LISTS] = { [0 ... PNODE_LISTS-1] = NULL };
/// Lock for the pnode lists
static lock_t pnode_lock = LOCK_INIT;

/** Check if larger free buddies are available */
static inline int pnode_large_avail(uint8_t exp)
{
	while (exp<PNODE_MAX && !pnode_lists[exp-PNODE_MIN])
		exp++;

	return exp != PNODE_MAX;
}

/** Calculate the required pnode size */
static inline int pnode_exp(size_t sz)
{
	int exp;
	for (exp=0; sz>(1<<exp); exp++);

	if (exp > PNODE_MAX)
		exp = 0;
	if (exp < PNODE_MIN)
		exp = PNODE_MIN;

	return exp;
}

/** Get a free pnode by potentially splitting a larger one */
static pnode_t* pnode_get(int exp)
{
	s_lock(&pnode_lock);
	pnode_t** list = &pnode_lists[exp-PNODE_MIN];
	pnode_t* pnode = *list;
	pnode_t* split;

	if (pnode)
		// there is already a free pnode =>
		// we remove it from the list
		*list = pnode->next;
	else if (exp >= PNODE_ALLOC && !pnode_large_avail(exp))
		// theres no free pnode larger than exp =>
		// we can allocate new memory
		pnode = (pnode_t*) palloc(1<<exp, 0);
	else {
		// we recursivly request a larger pnode...
		pnode = pnode_get(exp+1);
		if (BUILTIN_EXPECT(!pnode, 0))
			goto out;

		// ... and split it, by putting the second half back to the list
		split = (pnode_t*) ((size_t) pnode + (1<<exp));
		split->next = *list;
		*list = split;
	}

out:
	s_unlock(&pnode_lock);

	return pnode;
}

/** Put a pnode back to its free list
 */
static void pnode_put(pnode_t* pnode)
{
	s_lock(&pnode_lock);
	pnode_t** list = &pnode_lists[pnode->prefix.exponent-PNODE_MIN];
	pnode->next = *list;
	*list = pnode;
	s_unlock(&pnode_lock);
}

void pnode_dump(void)
{
	size_t free = 0;
	int i;
	for (i=0; i<PNODE_LISTS; i++) {
		pnode_t* pnode;
		int exp = i+PNODE_MIN;

		if (pnode_lists[i])
			kprintf("pnode_list[%d] (exp=%d, size=%d bytes):\n", i, exp, 1<<exp);

		for (pnode=pnode_lists[i]; pnode; pnode=pnode->next) {
			kprintf("  %x -> %x \n", pnode, pnode->next);
			free += 1<<exp;
		}
	}
	kprintf("free buddies: %d bytes\n", free);
}

void* palloc(size_t sz, uint32_t flags)
{
	size_t phyaddr, viraddr;
	uint32_t npages = PAGE_FLOOR(sz) >> PAGE_BITS;
	int err;

	//kprintf("palloc(%d) (%d pages)\n", sz, npages);

	// get free virtual address space
	viraddr = vma_alloc(npages*PAGE_SIZE, VMA_HEAP);
	if (BUILTIN_EXPECT(!viraddr, 0))
		return NULL;

	// get continous physical pages
	phyaddr = get_pages(npages);
	if (BUILTIN_EXPECT(!phyaddr, 0)) {
		vma_free(viraddr, viraddr+npages*PAGE_SIZE);
		return NULL;
	}

	// map physical pages to VMA
	err = page_map(viraddr, phyaddr, npages, PG_RW|PG_GLOBAL);
	if (BUILTIN_EXPECT(err, 0)) {
		vma_free(viraddr, viraddr+npages*PAGE_SIZE);
		put_pages(phyaddr, npages);
		return NULL;
	}

	return (void*) viraddr;
}

void pfree(void* addr, size_t sz)
{
	if (BUILTIN_EXPECT(!addr || !sz, 0))
		return;

	size_t i;
	size_t phyaddr;
	size_t viraddr = (size_t) addr & PAGE_MASK;
	uint32_t npages = PAGE_FLOOR(sz) >> PAGE_BITS;

	// memory is probably not continuously mapped! (userspace heap)
	for (i=0; i<npages; i++) {
		phyaddr = virt_to_phys(viraddr+i*PAGE_SIZE);
		put_page(phyaddr);
	}

	page_unmap(viraddr, npages);
	vma_free(viraddr, viraddr+npages*PAGE_SIZE);
}

void* kmalloc(size_t sz)
{
	if (BUILTIN_EXPECT(!sz, 0))
		return NULL;

	// add space for the prefix
	sz += sizeof(pnode_t);

	int exp = pnode_exp(sz);
	if (BUILTIN_EXPECT(!exp, 0))
		return NULL;

	pnode_t* pnode = pnode_get(exp);
	if (BUILTIN_EXPECT(!pnode, 0))
		return NULL;

	// setup pnode prefix
	pnode->prefix.magic = PNODE_MAGIC;
	pnode->prefix.exponent = exp;

	//kprintf("kmalloc(%d) = %x\n", sz, pnode+1);

	// pointer arithmetic: we hide the prefix
	return pnode+1;
}

void kfree(void *addr)
{
	if (BUILTIN_EXPECT(!addr, 0))
		return;

	//kprintf("kfree(%d)\n", addr);

	pnode_t* pnode = (pnode_t*) addr - 1; // get prefix

	// check magic
	if (BUILTIN_EXPECT(pnode->prefix.magic != PNODE_MAGIC, 0))
		return;

	pnode_put(pnode);
}

static size_t getsize(void* p) {
    size_t* in = (size_t*)p;
    if (in) {
        --in;
        return *in;
    }
    return -1;
}

void* realloc(void* ptr, size_t size) {
    void* newptr;
    size_t msize = getsize(ptr);
    if (size <= msize) return ptr;
    
    newptr = (void*)kmalloc(size);
    memcpy(newptr, ptr, msize);
    
    kfree(ptr);
    return newptr;
}
