#ifndef __MALLOC_H__
#define __MALLOC_H__

#include <sysdef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Binary exponent of maximal size for kmalloc()
#define PNODE_MAX	32 // 4 GB
/// Binary exponent of minimal pnode size
#define PNODE_MIN	3  // 8 Byte >= sizeof(pnode_t)
/// Binary exponent of the size which we allocate with pnode_fill()
#define PNODE_ALLOC	16 // 64 KByte = 16 * PAGE_SIZE

#define PNODE_LISTS	(PNODE_MAX-PNODE_MIN+1)
#define PNODE_MAGIC	0xBABE

union pnode;

/** Pnode
 *
 * Every free memory block is stored in a linked list according to its size.
 *  We can use this free memory to store store this pnode_t union which represents
 *  this block (the pnode_t union is alligned to the front).
 *  Therefore the address of the pnode_t union is equal with the address
 *  of the underlying free memory block.
 *
 * Every allocated memory block is prefixed with its binary size exponent and
 *  a known magic number. This prefix is hidden by the user because its located
 *  before the actual memory address returned by kmalloc()
 */
typedef union pnode {
	/// Pointer to the next pnode in the linked list.
	union pnode* next;
	struct {
		/// The binary exponent of the block size
		uint8_t exponent;
		/// Must be equal to PNODE_MAGIC for a valid memory block
		uint16_t magic;
	} prefix;
} pnode_t;

/** Dump free buddies */
void pnode_dump(void);

#ifdef __cplusplus
}
#endif

#endif

