#include <string.h>

/* Copies a string */
char *strcpy(char *dest, const char *src)
{
    do
    {
      *dest++ = *src++;
    }
    while (*src != 0);
    return dest;
}
