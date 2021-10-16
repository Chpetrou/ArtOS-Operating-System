#ifndef _STRING_H
#define _STRING_H 1
 
#include "sys/cdefs.h"
 
#include <sysdef.h>
 
#ifdef __cplusplus
extern "C" {
#endif
 
//String management functions
int memcmp(const void *ptr1, const void *ptr2, size_t len);
void *memcpy(void *dst, const void *src, size_t len);
void *memmove(void *dst, const void *src, size_t len);
void *memclr(void *ptr, size_t len);
void *memset(void *ptr, int value, size_t len);
size_t strlen(const char *str);
int strcmp(const char *str1, const char *str2);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
char* strchr (char * str, int character );
int strncmp (const char* s1, const char* s2, size_t len);
char* strncat (char* dest, const char* src, size_t len);
char* strncpy (char* dest, const char* src, size_t len);
size_t strnlen (const char* s, size_t len);
size_t strlcat (char* dest, const char* src, size_t len);
size_t strlcpy (char* dest, const char* src, size_t len);
    
#ifdef __cplusplus
}
#endif
 
#endif
