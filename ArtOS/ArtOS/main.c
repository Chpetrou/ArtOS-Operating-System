#include <sysdef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <tasks.h>
#include <processor.h>
#include <tasks.h>
#include <syscall.h>
#include <memory.h>
#include <vma.h>
#include <fs.h>
#include <terminal.h>
#include <kb.h>
#include <rtc.h>
#include <shell.h>

#include <lowlevel/irq.h>
#include <lowlevel/atomic.h>
#include <lowlevel/page.h>

/*
 * Note that linker symbols are not variables, they have no memory allocated for
 * maintaining a value, rather their address is their value.
 */
extern const void kernel_start;
extern const void kernel_end;
extern const void bss_start;
extern const void bss_end;
extern char __BUILD_DATE;
extern char __BUILD_TIME;

/* Page frame counters */
extern atomic_int32_t total_pages;
extern atomic_int32_t total_allocated_pages;
extern atomic_int32_t total_available_pages;


// Demo of a user-level task
static int NORETURN userfoo(void* arg)
{
//    char str[] = "hello from userfoo\n";
    char str[2];
    char ssr[] = "geia sas";

//    SYSCALL3(__NR_write, 0, str, 20);
    SYSCALL3(__NR_write, 0, ssr, 8);
    SYSCALL3(__NR_read, 1, str, 2);
    SYSCALL3(__NR_write, 0, str, 2);
    SYSCALL1(__NR_exit, 0);

    while(1) ;
}

//Test kernel function
static int test(void* arg)
{
    char str1[20], str2[30];
    
    kprintf("Enter name: ");
    kscanf("%s", str1);
    
    kprintf("Enter your website name: ");
    kscanf("%s", str2);
    
    kprintf("Entered Name: %s\n", str1);
    kprintf("Entered Website:%s", str2);
    
    return 0;
}

//Initialize ArtOS
static int ArtOS_init(void)
{
	// initialize .bss section
	memset((void*)&bss_start, 0x00, ((size_t) &bss_end - (size_t) &bss_start));

	monInit();
	system_init();
	irq_init();
    keyboard_install();
	timer_init();
    
    capsLock = 0;
    cursorY = 0;
    cursorX = 0;
    
	multitasking_init();
	memory_init();
	initrd_init();

	return 0;
}

//Main function of ArtOS
int main(void)
{
	ArtOS_init();
	system_calibration(); // enables also interrupts
    language = ENGLISH;
    artos_start();
    waitTicks(200);
    monInit();
    shell_main();

#if 0
	kputs("Filesystem:\n");
	list_fs(fs_root, 1);
#endif
    
	while(1) { 
		HALT;
	}

	return 0;
}
