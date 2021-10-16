#include <string.h>

/* Compine a string part */
size_t strlcat (char* dest, const char* src, size_t len)
{
	size_t destlen = strnlen (dest, len);
	if (destlen > len - 1)
	{
		return destlen + strlen (src);
	}
	return destlen + strlcpy (dest + destlen, src, len - destlen);
}
