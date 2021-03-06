#include <sysdef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <processor.h>
#include <time.h>
#include <spinlock.h>
#include <vma.h>
#include <lowlevel/irq.h>
#include <lowlevel/idt.h>
#include <lowlevel/irqflags.h>
#include <lowlevel/io.h>
#include <lowlevel/page.h>
#include <lowlevel/apic.h>
#include <lowlevel/multiboot.h>

/*
 * Note that linker symbols are not variables, they have no memory allocated for
 * maintaining a value, rather their address is their value.
 */
extern const void kernel_start;

#define IOAPIC_ADDR	((size_t) &kernel_start - 1*PAGE_SIZE)
#define LAPIC_ADDR	((size_t) &kernel_start - 2*PAGE_SIZE)

// IO APIC MMIO structure: write reg, then read or write data.
typedef struct {
	uint32_t reg;
	uint32_t pad[3];
	uint32_t data;
} ioapic_t;

#ifndef MAX_CORES
#define MAX_CORES	4
#endif

static const apic_processor_entry_t* apic_processors[MAX_CORES] = {[0 ... MAX_CORES-1] = NULL};
static uint32_t boot_processor = MAX_CORES;
apic_mp_t* apic_mp  __attribute__ ((section (".data"))) = NULL;
static apic_config_table_t* apic_config = NULL;
static size_t lapic = 0;
static volatile ioapic_t* ioapic = NULL;
static uint32_t icr = 0;
static uint32_t ncores = 1;
static uint8_t irq_redirect[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
static uint8_t initialized = 0;
lock_t bootlock = LOCK_INIT;

// forward declaration
static int lapic_reset(void);

static uint32_t lapic_read_default(uint32_t addr)
{
	return *((const volatile uint32_t*) (lapic+addr));
}

static uint32_t lapic_read_msr(uint32_t addr)
{
	return rdmsr(0x800 + (addr >> 4));
}

typedef uint32_t (*lapic_read_func)(uint32_t addr);

static lapic_read_func lapic_read = lapic_read_default;

static void lapic_write_default(uint32_t addr, uint32_t value)
{
	/*
	 * to avoid a pentium bug, we have to read a apic register
	 * before we write a value to this register
	 */
	asm volatile ("movl (%%eax), %%edx; movl %%ebx, (%%eax)" :: "a"(lapic+addr), "b"(value) : "%edx");
}

static void lapic_write_msr(uint32_t addr, uint32_t value)
{
	wrmsr(0x800 + (addr >> 4), value);
}

typedef void (*lapic_write_func)(uint32_t addr, uint32_t value);

static lapic_write_func lapic_write = lapic_write_default;

static inline uint32_t ioapic_read(uint32_t reg)
{
	ioapic->reg = reg;

	return ioapic->data;
}

static inline void ioapic_write(uint32_t reg, uint32_t value)
{
	ioapic->reg = reg;
	ioapic->data = value;
}

static inline uint32_t ioapic_version(void)
{
	if (ioapic)
		return ioapic_read(IOAPIC_REG_VER) & 0xFF;

	return 0;
}

static inline uint32_t ioapic_max_redirection_entry(void)
{
	if (ioapic)
		 return (ioapic_read(IOAPIC_REG_VER) >> 16) & 0xFF;

	return 0;
}

/*
 * Send a 'End of Interrupt' command to the APIC
 */
void apic_eoi(void)
{
	lapic_write(APIC_EOI, 0);
}

int apic_is_enabled(void)
{
	return (lapic && initialized);
}

uint32_t apic_cpu_id(void)
{
	if (apic_is_enabled())
		return ((lapic_read(APIC_ID)) >> 24);

	return 0;
}

static inline void apic_set_cpu_id(uint32_t id)
{
	if (apic_is_enabled())
		lapic_write(APIC_ID, id << 24);
}

static inline uint32_t apic_version(void)
{
	if (lapic)
		return lapic_read(APIC_VERSION) & 0xFF;

	return 0;
}

static inline uint32_t apic_lvt_entries(void)
{
	if (lapic)
		return (lapic_read(APIC_VERSION) >> 16) & 0xFF;

	return 0;
}

int apic_disable_timer(void)
{
	if (BUILTIN_EXPECT(!apic_is_enabled(), 0))
		return -EINVAL;

	lapic_write(APIC_LVT_T, 0x10000);	// disable timer interrupt

	return 0;
}

int apic_enable_timer(void)
{
	if (BUILTIN_EXPECT(apic_is_enabled() && icr, 1)) {
		lapic_write(APIC_DCR, 0xB);		// set it to 1 clock increments
		lapic_write(APIC_LVT_T, 0x2007B);	// connects the timer to 123 and enables it
		lapic_write(APIC_ICR, icr);

		return 0;
	}

	return -EINVAL;
}

static apic_mp_t* search_mptable(size_t base, size_t limit) {
	size_t ptr=PAGE_CEIL(base), vptr=0;
	apic_mp_t* tmp;
	uint32_t i;

	while(ptr<=limit-sizeof(apic_mp_t)) {
		if (vptr) {
			// unmap page via mapping a zero page
			page_unmap(vptr, 1);
			vptr = 0;
		}

		if (BUILTIN_EXPECT(!page_map(ptr & PAGE_MASK, ptr & PAGE_MASK, 1, PG_GLOBAL | PG_RW | PG_PCD), 1))
			vptr = ptr & PAGE_MASK;
		else
			return NULL;

		for(i=0; (vptr) && (i<PAGE_SIZE-sizeof(apic_mp_t)); i+=4, vptr+=4) {
			tmp = (apic_mp_t*) vptr;
			if (tmp->signature == MP_TABLE_SIG) {
				if (!((tmp->version > 4) || (tmp->features[0]))) {
					vma_add(ptr & PAGE_MASK, (ptr & PAGE_MASK) + PAGE_SIZE, VMA_READ|VMA_WRITE);
					return tmp;
				}
			}
		}

		ptr += PAGE_SIZE;
	}

	if (vptr) {
		// unmap page via mapping a zero page
		page_unmap(vptr, 1);
	}

	return NULL;
}

static int lapic_reset(void)
{
	uint32_t max_lvt;

	if (!lapic)
		return -ENXIO;

	max_lvt = apic_lvt_entries();

	lapic_write(APIC_SVR, 0x17F);	// enable the apic and connect to the idt entry 127
	lapic_write(APIC_TPR, 0x00);	// allow all interrupts
	if (icr) {
		lapic_write(APIC_DCR, 0xB);		// set it to 1 clock increments
		lapic_write(APIC_LVT_T, 0x2007B);	// connects the timer to 123 and enables it
		lapic_write(APIC_ICR, icr);
	} else
		lapic_write(APIC_LVT_T, 0x10000);	// disable timer interrupt
	if (max_lvt >= 4)
		lapic_write(APIC_LVT_TSR, 0x10000);	// disable thermal sensor interrupt
	if (max_lvt >= 5)
		lapic_write(APIC_LVT_PMC, 0x10000);	// disable performance counter interrupt
	lapic_write(APIC_LINT0, 0x7C);	// connect LINT0 to idt entry 124
	lapic_write(APIC_LINT1, 0x7D);	// connect LINT1 to idt entry 125
	lapic_write(APIC_LVT_ER, 0x7E);	// connect error to idt entry 126

	return 0;
}

/*
 * detects the timer frequency of the APIC and restart
 * the APIC timer with the correct period
 */
int apic_calibration(void)
{
	uint32_t i;
	uint32_t flags;
	uint64_t ticks, old;

	if (!lapic)
		return -ENXIO;

	old = get_clock_tick();

	/* wait for the next time slice */
	while ((ticks = get_clock_tick()) - old == 0)
		HALT;

	flags = irq_nested_disable();
	lapic_write(APIC_DCR, 0xB);			// set it to 1 clock increments
	lapic_write(APIC_LVT_T, 0x2007B); 	// connects the timer to 123 and enables it
	lapic_write(APIC_ICR, 0xFFFFFFFFUL);
	irq_nested_enable(flags);

	/* wait 3 time slices to determine a ICR */
	while (get_clock_tick() - ticks < 3)
		HALT;

	icr = (0xFFFFFFFFUL - lapic_read(APIC_CCR)) / 3;

	flags = irq_nested_disable();
	lapic_reset();
	irq_nested_enable(flags);

	// Now, ArtOS is able to use the APIC => Therefore, we disable the PIC
	outportb(0xA1, 0xFF);
	outportb(0x21, 0xFF);

//    kprintf("APIC calibration determines an ICR of 0x%x\n", icr);

	flags = irq_nested_disable();

	if (ioapic) {
		uint32_t max_entry = ioapic_max_redirection_entry();

		// now lets turn everything else on
		for(i=0; i<=max_entry; i++)
			if (i != 2)
				ioapic_inton(i, apic_processors[boot_processor]->id);
		// now, we don't longer need the IOAPIC timer and turn it off
		ioapic_intoff(2, apic_processors[boot_processor]->id);
	}

	initialized = 1;
	irq_nested_enable(flags);

	return 0;
}

static int apic_probe(void)
{
	size_t addr;
	uint32_t i, j, count;
	int isa_bus = -1;

	apic_mp = search_mptable(0xF0000, 0x100000);
	if (apic_mp)
		goto found_mp;
	apic_mp = search_mptable(0x9F000, 0xA0000);
	if (apic_mp)
		goto found_mp;

found_mp:
	if (!apic_mp)
		goto no_mp;

//    kprintf("Found MP config table at 0x%x\n", apic_mp->mp_config);
//    kprintf("System uses Multiprocessing Specification 1.%d\n", apic_mp->version);
//    kprintf("MP features 1: %d\n", apic_mp->features[0]);

	if (apic_mp->features[0]) {
//        kputs("Currently, ArtOS supports only multiprocessing via the MP config tables!\n");
		goto no_mp;
	}

//    if (apic_mp->features[1] & 0x80)
//        kputs("PIC mode implemented\n");
//    else
//        kputs("Virtual-Wire mode implemented\n");

	apic_config = (apic_config_table_t*) ((size_t) apic_mp->mp_config);
	if (((size_t) apic_config & PAGE_MASK) != ((size_t) apic_mp & PAGE_MASK)) {
		page_map((size_t) apic_config & PAGE_MASK,  (size_t) apic_config & PAGE_MASK, 1, PG_GLOBAL | PG_RW | PG_PCD);
		vma_add( (size_t) apic_config & PAGE_MASK, ((size_t) apic_config & PAGE_MASK) + PAGE_SIZE, VMA_READ|VMA_WRITE);
	}

	if (!apic_config || strncmp((void*) &apic_config->signature, "PCMP", 4) !=0) {
		kputs("Invalid MP config table\n");
		goto no_mp;
	}

	addr = (size_t) apic_config;
	addr += sizeof(apic_config_table_t);

	// search the ISA bus => required to redirect the IRQs
	for(i=0; i<apic_config->entry_count; i++) {
		switch(*((uint8_t*) addr)) {
		case 0:
			addr += 20;
			break;
		case 1: {
				apic_bus_entry_t* mp_bus;

				mp_bus = (apic_bus_entry_t*) addr;
				if (mp_bus->name[0] == 'I' && mp_bus->name[1] == 'S' &&
				    mp_bus->name[2] == 'A')
					isa_bus = i;
			}
			addr += 8;
			break;
		default:
			addr += 8;
		}
	}

	addr = (size_t) apic_config;
	addr += sizeof(apic_config_table_t);

	for(i=0, j=0, count=0; i<apic_config->entry_count; i++) {
		if (*((uint8_t*) addr) == 0) { // cpu entry
			apic_processor_entry_t* cpu = (apic_processor_entry_t*) addr;

			if (j < MAX_CORES) {
				 // is the processor usable?
				if (cpu->cpu_flags & 0x01) {
					apic_processors[j] = cpu;
					if (cpu->cpu_flags & 0x02)
						boot_processor = j;
					j++;
				}
			}

			if (cpu->cpu_flags & 0x01)
				count++;
			addr += 20;
		} else if (*((uint8_t*) addr) == 2) { // IO_APIC
			apic_io_entry_t* io_entry = (apic_io_entry_t*) addr;
			ioapic = (ioapic_t*) ((size_t) io_entry->addr);
//            kprintf("Found IOAPIC at 0x%x\n", ioapic);
			page_map(IOAPIC_ADDR, (size_t)ioapic & PAGE_MASK, 1, PG_GLOBAL | PG_RW | PG_PCD);
			vma_add(IOAPIC_ADDR, IOAPIC_ADDR + PAGE_SIZE, VMA_READ|VMA_WRITE);
			ioapic = (ioapic_t*) IOAPIC_ADDR;
			addr += 8;
//            kprintf("Map IOAPIC to 0x%x\n", ioapic);
		} else if (*((uint8_t*) addr) == 3) { // IO_INT
			apic_ioirq_entry_t* extint = (apic_ioirq_entry_t*) addr;
			if (extint->src_bus == isa_bus) {
				irq_redirect[extint->src_irq] = extint->dest_intin;
				kprintf("Redirect irq %d -> %d\n", extint->src_irq,  extint->dest_intin);
			}
			addr += 8;
		} else addr += 8;
	}
//    kprintf("Found %d cores\n", count);

	if (count > MAX_CORES) {
		kputs("Found too many cores! Increase the macro MAX_CORES!\n");
		goto no_mp;
	}
	ncores = count;

check_lapic:
	if (apic_config)
		lapic = apic_config->lapic;
	else if (has_apic())
		lapic = 0xFEE00000;

	if (!lapic)
		goto out;
//    kprintf("Found APIC at 0x%x\n", lapic);

	if (has_x2apic()) {
		kprintf("Enable X2APIC support!\n");
		wrmsr(0x1B, lapic | 0xD00);
		lapic_read = lapic_read_msr;
		lapic_write = lapic_write_msr;
	} else {
		page_map(LAPIC_ADDR, (size_t)lapic & PAGE_MASK, 1, PG_GLOBAL | PG_RW | PG_PCD);
		vma_add(LAPIC_ADDR, LAPIC_ADDR + PAGE_SIZE, VMA_READ | VMA_WRITE);
		lapic = LAPIC_ADDR;
//        kprintf("Map APIC to 0x%x\n", lapic);
	}

//    kprintf("Maximum LVT Entry: 0x%x\n", apic_lvt_entries());
//    kprintf("APIC Version: 0x%x\n", apic_version());

	if (!((apic_version() >> 4))) {
		kprintf("Currently, ArtOS didn't supports extern APICs!\n");
		goto out;
	}

	if (apic_lvt_entries() < 3) {
		kprintf("LVT is too small\n");
		goto out;
	}

	return 0;

out:
	apic_mp = NULL;
	apic_config = NULL;
	lapic = 0;
	ncores = 1;
	return -ENXIO;

no_mp:
	apic_mp = NULL;
	apic_config = NULL;
	ncores = 1;
	goto check_lapic;
}

static void apic_err_handler(struct registers *s)
{
	kprintf("Got APIC error 0x%x\n", lapic_read(APIC_ESR));
}

int apic_init(void)
{
	int ret;

	ret = apic_probe();
	if (ret)
		return ret;

	// set APIC error handler
	irq_install_handler(126, apic_err_handler);
//    kprintf("Boot processor %d (ID %d)\n", boot_processor, apic_processors[boot_processor]->id);

	return 0;
}

int ioapic_inton(uint8_t irq, uint8_t apicid)
{
	ioapic_route_t route;
	uint32_t off;

	if (BUILTIN_EXPECT(irq > 24, 0)){
		kprintf("IOAPIC: trying to turn on irq %i which is too high\n", irq);
		return -EINVAL;
	}

	if (irq < 16)
		off = irq_redirect[irq]*2;
	else
		off = irq*2;
#if 0
	route.lower.whole = ioapic_read(IOAPIC_REG_TABLE+1+off);
	route.dest.upper = ioapic_read(IOAPIC_REG_TABLE+off);
	route.lower.bitfield.mask = 0; // turn it on (stop masking)
#else
	route.lower.bitfield.dest_mode = 0;
	route.lower.bitfield.mask = 0;
	route.dest.physical.physical_dest = apicid; // send to the boot processor
	route.lower.bitfield.delivery_mode = 0;
	route.lower.bitfield.polarity = 0;
	route.lower.bitfield.trigger = 0;
	route.lower.bitfield.vector = 0x20+irq;
	route.lower.bitfield.mask = 0; // turn it on (stop masking)
#endif

	ioapic_write(IOAPIC_REG_TABLE+off, route.lower.whole);
	ioapic_write(IOAPIC_REG_TABLE+1+off, route.dest.upper);

	route.dest.upper = ioapic_read(IOAPIC_REG_TABLE+1+off);
        route.lower.whole = ioapic_read(IOAPIC_REG_TABLE+off);

	return 0;
}

int ioapic_intoff(uint8_t irq, uint8_t apicid)
{
	ioapic_route_t route;
	uint32_t off;

	if (BUILTIN_EXPECT(irq > 24, 0)){
		kprintf("IOAPIC: trying to turn on irq %i which is too high\n", irq);
		return -EINVAL;
	}

	if (irq < 16) 
		off = irq_redirect[irq]*2;
	else
		off = irq*2;

#if 0
	route.lower.whole = ioapic_read(IOAPIC_REG_TABLE+1+off);
	route.dest.upper = ioapic_read(IOAPIC_REG_TABLE+off);
	route.lower.bitfield.mask = 1; // turn it off (start masking)
#else
	route.lower.bitfield.dest_mode = 0;
	route.lower.bitfield.mask = 0;
	route.dest.physical.physical_dest = apicid;
	route.lower.bitfield.delivery_mode = 0;
	route.lower.bitfield.polarity = 0;
	route.lower.bitfield.trigger = 0;
	route.lower.bitfield.vector = 0x20+irq;
	route.lower.bitfield.mask = 1; // turn it off (start masking)
#endif

	ioapic_write(IOAPIC_REG_TABLE+off, route.lower.whole);
	ioapic_write(IOAPIC_REG_TABLE+1+off, route.dest.upper);

	return 0;
}
