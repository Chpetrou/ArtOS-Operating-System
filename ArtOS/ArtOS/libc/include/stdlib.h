#ifndef _STDLIB_H
#define _STDLIB_H 1
 
#include "sys/cdefs.h"
#include <sysdef.h>
 
#ifdef __cplusplus
extern "C" {
#endif
 
//Stdlib functions
__attribute__((__noreturn__))
void abort(void);
char *strstr(const char *s, const char *find);
long strtol(const char* str, char** endptr, int base);
unsigned long strtoul(const char* nptr, char** endptr, int base);
int atoi(const char * s);
void* palloc(size_t sz, uint32_t flags);
void pfree(void* addr, size_t sz);
void* kmalloc(size_t sz);
void kfree(void* addr);
void* realloc(void* ptr, size_t size);
void* create_stack(tid_t id);
 
#ifdef __cplusplus
}
#endif
 
#endif
