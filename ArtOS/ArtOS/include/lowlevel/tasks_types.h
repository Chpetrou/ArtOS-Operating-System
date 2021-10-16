#ifndef __ASM_TASKS_TYPES_H__
#define __ASM_TASKS_TYPES_H__

#include <sysdef.h>
#include <lowlevel/processor.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	long	cwd;
	long	swd;
	long	twd;
	long	fip;
	long	fcs;
	long	foo;
	long	fos;
	long	st_space[20];
	long	status;
} i387_fsave_t;

typedef struct i387_fxsave_struct {
	unsigned short	cwd;
	unsigned short	swd;
	unsigned short	twd;
	unsigned short	fop;
	long	fip;
	long	fcs;
	long	foo;
	long	fos;
	long	mxcsr;
	long	mxcsr_mask;
	long	st_space[32];
	long	xmm_space[64];
	long	padding[12];
	union {
		long	padding1[12];
		long	sw_reserved[12];
	};
} i387_fxsave_t __attribute__ ((aligned (16)));

union fpu_state {
	i387_fsave_t	fsave;
	i387_fxsave_t	fxsave;
};

typedef void (*handle_fpu_state)(union fpu_state* state);

extern handle_fpu_state save_fpu_state;
extern handle_fpu_state restore_fpu_state;
extern handle_fpu_state fpu_init;

#ifdef __cplusplus
}
#endif

#endif
