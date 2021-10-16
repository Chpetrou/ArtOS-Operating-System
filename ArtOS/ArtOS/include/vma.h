#ifndef __VMA_H__
#define __VMA_H__

#include <sysdef.h>
#include <lowlevel/page.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Read access to this VMA is allowed
#define VMA_READ	(1 << 0)
/// Write access to this VMA is allowed
#define VMA_WRITE	(1 << 1)
/// Instructions fetches in this VMA are allowed
#define VMA_EXECUTE	(1 << 2)
/// This VMA is cacheable
#define VMA_CACHEABLE	(1 << 3)
/// This VMA is not accessable
#define VMA_NO_ACCESS	(1 << 4)
/// This VMA should be part of the userspace
#define VMA_USER	(1 << 5)
/// A collection of flags used for the kernel heap (kmalloc)
#define VMA_HEAP	(VMA_READ|VMA_WRITE|VMA_CACHEABLE)

// boundaries for VAS allocation
#define VMA_KERN_MIN	0xC0000
#define VMA_KERN_MAX	KERNEL_SPACE
#define VMA_USER_MIN	KERNEL_SPACE

// last three top level entries are reserved
 #define VMA_USER_MAX	0xFF400000

struct vma;

/** VMA structure definition
 *
 * Each item in this linked list marks a used part of the virtual address space.
 * Its used by vm_alloc() to find holes between them.
 */
typedef struct vma {
	/// Start address of the memory area
	size_t start;
	/// End address of the memory area
	size_t end;
	/// Type flags field
	uint32_t flags;
	/// Pointer of next VMA element in the list
	struct vma* next;
	/// Pointer to previous VMA element in the list
	struct vma* prev;
} vma_t;

/** Initalize the kernelspace VMA list
 *
 * Reserves several system-relevant virtual memory regions:
 *  - SMP boot page (SMP_SETUP_ADDR)
 *  - VGA video memory (VIDEO_MEM_ADDR)
 *  - The kernel (kernel_start - kernel_end)
 *  - Multiboot structure (mb_info)
 *  - Multiboot mmap (mb_info->mmap_*)
 *  - Multiboot modules (mb_info->mods_*)
 *    - Init Ramdisk
 *
 * 
 *  - 0 on success
 *  - <0 on failure
 */
int vma_init(void);

/** Add a new virtual memory area to the list of VMAs 
 *
 * start Start address of the new area
 * end End address of the new area
 * flags Type flags the new area shall have
 *
 * 
 * - 0 on success
 * - -EINVAL (-22) or -EINVAL (-12) on failure
 */
int vma_add(size_t start, size_t end, uint32_t flags);

/** Search for a free memory area
 *
 * size Size of requestes VMA in bytes
 * flags
 * Type flags the new area shall have
 * - 0 on failure
 * - the start address of a free area
 */
size_t vma_alloc(size_t size, uint32_t flags);

/** Free an allocated memory area
 *
 * start Start address of the area to be freed
 * end End address of the to be freed
 * 
 * - 0 on success
 * - -EINVAL (-22) on failure
 */
int vma_free(size_t start, size_t end);

/** Free all virtual memory areas
 *
 * 
 * - 0 on success
 */
int drop_vma_list(struct task* task);

/** Copy the VMA list of the current task to task
 *
 * task The task where the list should be copied to
 * 
 * - 0 on success
 */
int copy_vma_list(struct task* src, struct task* dest);

/** Dump information about this task's VMAs into the terminal. */
void vma_dump(void);

#ifdef __cplusplus
}
#endif

#endif
