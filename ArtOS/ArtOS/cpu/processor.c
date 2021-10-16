#include <sysdef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <processor.h>
#include <tasks.h>

//Useful functions needed later
extern void isrsyscall(void);

cpu_info_t cpu_info = {0, 0, 0, 0};
static uint32_t cpu_freq = 0;

static void default_mb(void)
{
	asm volatile ("lock; addl $0,0(%%esp)" ::: "memory", "cc");
}

static void default_save_fpu_state(union fpu_state* state)
{
	asm volatile ("fnsave %0; fwait" : "=m"((*state).fsave) :: "memory");
}

static void default_restore_fpu_state(union fpu_state* state)
{
	asm volatile ("frstor %0" :: "m"(state->fsave));
}

static void default_fpu_init(union fpu_state* fpu)
{
	i387_fsave_t *fp = &fpu->fsave;

	memset(fp, 0x00, sizeof(i387_fsave_t));
	fp->cwd = 0xffff037fu;
	fp->swd = 0xffff0000u;
	fp->twd = 0xffffffffu;
	fp->fos = 0xffff0000u;
}

func_memory_barrier mb = default_mb;
func_memory_barrier rmb = default_mb;
func_memory_barrier wmb = default_mb;

static void mfence(void) { asm volatile("mfence" ::: "memory"); }
static void lfence(void) { asm volatile("lfence" ::: "memory"); }
static void sfence(void) { asm volatile("sfence" ::: "memory"); }
handle_fpu_state save_fpu_state = default_save_fpu_state;
handle_fpu_state restore_fpu_state = default_restore_fpu_state;
handle_fpu_state fpu_init = default_fpu_init;

static void save_fpu_state_fxsr(union fpu_state* state)
{
	asm volatile ("fxsave %0; fnclex" : "=m"((*state).fxsave) :: "memory");
}

static void restore_fpu_state_fxsr(union fpu_state* state)
{
	asm volatile ("fxrstor %0" :: "m"(state->fxsave));
}

static void fpu_init_fxsr(union fpu_state* fpu)
{
	i387_fxsave_t* fx = &fpu->fxsave;

	memset(fx, 0x00, sizeof(i387_fxsave_t));
	fx->cwd = 0x37f;
	if (BUILTIN_EXPECT(has_sse(), 1))
		fx->mxcsr = 0x1f80;
}

//Detects the frequency of the CPU
uint32_t detect_cpu_frequency(void)
{
	uint64_t start, end, diff;
	uint64_t ticks, old;

	if (BUILTIN_EXPECT(cpu_freq > 0, 0))
		return cpu_freq;

	old = get_clock_tick();

	/* wait for the next time slice */
	while((ticks = get_clock_tick()) - old == 0)
		HALT;

	rmb();
	start = rdtsc();
	/* wait a second to determine the frequency */
	while(get_clock_tick() - ticks < TIMER_FREQ)
		HALT;
	rmb();
	end = rdtsc();

	diff = end > start ? end - start : start - end;
	cpu_freq = (uint32_t) (diff / (uint64_t) 1000000);

	return cpu_freq;
}

//Detects what model is the CPU
int cpu_detection(void)
{
	uint32_t a=0, b=0, c=0, d=0;
	uint32_t family, model, stepping;
	size_t cr4;
	uint8_t first_time = 0;

	if (!cpu_info.feature1)
    {
		first_time = 1;
		cpuid(1, &a, &b, &cpu_info.feature2, &cpu_info.feature1);

		family   = (a & 0x00000F00) >> 8;
		model    = (a & 0x000000F0) >> 4;
		stepping =  a & 0x0000000F;
		if ((family == 6) && (model < 3) && (stepping < 3))
			cpu_info.feature1 &= ~CPU_FEATURE_SEP;

		cpuid(0x80000001, &a, &b, &c, &cpu_info.feature3);
		cpuid(0x80000008, &cpu_info.addr_width, &b, &c, &d);
	}

	cr4 = read_cr4();
	if (has_fxsr())
		cr4 |= CR4_OSFXSR;		// set the OSFXSR bit
	if (has_sse())
		cr4 |= CR4_OSXMMEXCPT;	// set the OSXMMEXCPT bit
	if (has_pge())
		cr4 |= CR4_PGE;
	write_cr4(cr4);

	if (first_time && has_sse())
		wmb = sfence;

	if (first_time && has_sse2()) {
		rmb = lfence;
		mb = mfence;
	}

	if (has_fpu()) {
		if (first_time)
		asm volatile ("fninit");
	}

	if (first_time && has_fxsr()) {
		save_fpu_state = save_fpu_state_fxsr;
		restore_fpu_state = restore_fpu_state_fxsr;
		fpu_init = fpu_init_fxsr;
	}

	if (first_time && on_hypervisor()) {
		uint32_t c, d;
		char vendor_id[13];

		cpuid(0x40000000, &a, &b, &c, &d);
		memcpy(vendor_id, &b, 4);
		memcpy(vendor_id + 4, &c, 4);
		memcpy(vendor_id + 8, &d, 4);
		vendor_id[12] = '\0';
	}

	return 0;
}

//Gets the frequency of the CPU
uint32_t get_cpu_frequency(void)
{	
	if (cpu_freq > 0)
		return cpu_freq;

	return detect_cpu_frequency();
}

