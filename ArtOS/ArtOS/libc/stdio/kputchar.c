#include <stdio.h>
 
#if defined(__is_libk)
#include <kernel/tty.h>
#endif
 
int kputchar(int ic) {
#if defined(__is_libk)
	char c = (char) ic;
	terminal_write(&c, sizeof(c));
#endif
	return ic;
}
