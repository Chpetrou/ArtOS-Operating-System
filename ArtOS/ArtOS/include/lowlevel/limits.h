#ifndef __ARCH_LIMITS_H__
#define __ARCH_LIMITS_H__

#ifdef __cplusplus
extern "C" {
#endif

/** Number of bits in a char */
#define	CHAR_BIT	8		

/** Maximum value for a signed char */
#define	SCHAR_MAX	0x7f		
/** Minimum value for a signed char */
#define	SCHAR_MIN	(-0x7f - 1)	

/** Maximum value for an unsigned char */
#define	UCHAR_MAX	0xff		

/** Maximum value for an unsigned short */
#define	USHRT_MAX	0xffff		
/** Maximum value for a short */
#define	SHRT_MAX	0x7fff		
/** Minimum value for a short */
#define	SHRT_MIN	(-0x7fff - 1)	

/** Maximum value for an unsigned int */
#define	UINT_MAX	0xffffffffU	
/** Maximum value for an int */
#define	INT_MAX		0x7fffffff	
/** Minimum value for an int */
#define	INT_MIN	(-0x7fffffff - 1)	

/** Maximum value for an unsigned long */
#define	ULONG_MAX	0xffffffffUL	
/** Maximum value for a long */
#define	LONG_MAX	0x7fffffffL	
/** Minimum value for a long */
#define	LONG_MIN	(-0x7fffffffL - 1)	

/** Maximum value for an unsigned long long */
#define	ULLONG_MAX	0xffffffffffffffffULL
/** Maximum value for a long long */
#define	LLONG_MAX	0x7fffffffffffffffLL	
/** Minimum value for a long long */
#define	LLONG_MIN	(-0x7fffffffffffffffLL - 1)  

#ifdef __cplusplus
}
#endif

#endif
