#ifndef __ARCH_ATOMIC_H__
#define __ARCH_ATOMIC_H__

#include <sysdef.h>

#ifdef __cplusplus
extern "C" {
#endif

#if MAX_CORES > 1
#define LOCK "lock ; "
#else
#define LOCK ""
#endif

/** Makro for initialization of atomic_int32_t vars
 *
 * Whenever you use an atomic_int32_t variable, init it with 
 * this macro first.\n
 * Example: atomic_int32_t myAtomicVar = ATOMIC_INIT(123);
 *
 * i The number value you want to init it with.
 */
#define ATOMIC_INIT(i)  { (i) }

/** Standard-datatype for atomic operations
 *
 * It just consists of an int32_t variable internally, marked as volatile.
 */
typedef struct { volatile int32_t counter; } atomic_int32_t;

/** Atomic test and set operation for int32 vars.
 *
 * This function will atomically exchange the value of an atomic variable and
 * return its old value. Is used in locking-operations.\n
 * \n
 * Intel manuals: If a memory operand is referenced, the processor's locking 
 * protocol is automatically implemented for the duration of the exchange 
 * operation, regardless of the presence or absence of the LOCK prefix.
 *
 * d Pointer to the atomic_int_32_t with the value you want to exchange
 * ret the value you want the var test for
 *
 * The old value of the atomic_int_32_t var before exchange
 */
inline static int32_t atomic_int32_test_and_set(atomic_int32_t* d, int32_t ret)
{
	asm volatile ("xchgl %0, %1" : "=r"(ret) : "m"(d->counter), "0"(ret) : "memory");
	return ret;
}

/** Atomic addition of values to atomic_int32_t vars
 *
 * This function lets you add values in an atomic operation
 *
 * d Pointer to the atomit_int32_t var you want do add a value to
 * i The value you want to increment by
 *
 * The mathematical result
 */
inline static int32_t atomic_int32_add(atomic_int32_t *d, int32_t i)
{
	int32_t res = i;
	asm volatile(LOCK "xaddl %0, %1" : "=r"(i) : "m"(d->counter), "0"(i) : "memory", "cc");
	return res+i;
}

/** Atomic subtraction of values from atomic_int32_t vars
 *
 * This function lets you subtract values in an atomic operation.\n
 * This function is just for convenience. It uses atomic_int32_add(d, -i)
 *
 * d Pointer to the atomic_int32_t var you want to subtract from
 * i The value you want to subtract by
 *
 * The mathematical result
 */
inline static int32_t atomic_int32_sub(atomic_int32_t *d, int32_t i)
{
        return atomic_int32_add(d, -i);
}

/** Atomic increment by one
 *
 * The atomic_int32_t var will be atomically incremented by one.\n
 *
 * d The atomic_int32_t var you want to increment
 */
inline static int32_t atomic_int32_inc(atomic_int32_t* d) {
	return atomic_int32_add(d, 1);
}

/** Atomic decrement by one
 *
 * The atomic_int32_t var will be atomically decremented by one.\n
 *
 * d The atomic_int32_t var you want to decrement
 */
inline static int32_t atomic_int32_dec(atomic_int32_t* d) {
	return atomic_int32_add(d, -1);
}

/** Read out an atomic_int32_t var
 *
 * This function is for convenience: It looks into the atomic_int32_t struct
 * and returns the internal value for you.
 *
 * d Pointer to the atomic_int32_t var you want to read out
 * It's number value
 */
inline static int32_t atomic_int32_read(atomic_int32_t *d) {
	return d->counter;
}

/** Set the value of an atomic_int32_t var
 *
 * This function is for convenience: It sets the internal value of 
 * an atomic_int32_t var for you.
 *
 * d Pointer to the atomic_int32_t var you want to set
 * v The value to set
 */
inline static void atomic_int32_set(atomic_int32_t *d, int32_t v) {
	atomic_int32_test_and_set(d, v);
}

#ifdef __cplusplus
}
#endif

#endif
