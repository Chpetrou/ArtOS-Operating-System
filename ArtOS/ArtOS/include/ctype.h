#ifndef __CTYPE_H_
#define __CYTPE_H_

/** Returns true if the value of 'c' is an ASCII-charater */
static inline int isascii(int c) 
{
	return (((unsigned char)(c))<=0x7f);
}

/** Applies an and-operation to 
 * push the value of 'c' into the ASCII-range */
static inline int toascii(int c)
{
	return (((unsigned char)(c))&0x7f);
}

/** Returns true if the value of 'c' is the 
 * space character or a control character */
static inline int iswhitespace(int c)
{
	if (!isascii(c))
		return 0;

	if (' ' == (unsigned char) c)
		return 1;
	if ('\n' == (unsigned char) c)
		return 1;
	if ('\r' == (unsigned char) c)
		return 1;
	if ('\t' == (unsigned char) c)
		return 1;
	if ('\v' == (unsigned char) c)
		return 1;
	if ('\f' == (unsigned char) c)
		return 1;

	return 0;
}

/** Returns true if the value of 'c' is a number */
static inline int isdigit(int c)
{
	if (!isascii(c))
		return 0;

	if (((unsigned char) c >= '0') && ((unsigned char) c <= '9'))
		return 1;

	return 0;
}

/** Returns true if the value of 'c' is a lower case letter */
static inline int islower(int c)
{
	if (!isascii(c))
		return 0;

	if (((unsigned char) c >= 'a') && ((unsigned char) c <= 'z'))
		return 1;

	return 0;
}

/** Returns true if the value of 'c' is an upper case letter */
static inline int isupper(int c)
{
	if (!isascii(c))
		return 0;

	if (((unsigned char) c >= 'A') && ((unsigned char) c <= 'Z'))
		return 1;

	return 0;
}

/** Returns true if the value of 'c' is an alphabetic character */
static inline int isalpha(int c)
{
	if (isupper(c) || islower(c))
		return 1;

	return 0;
}

/** Makes the input character lower case.\n Will do nothing if it 
 * was something different than an upper case letter before. */
static inline unsigned char tolower(unsigned char c)
{
	if (isupper(c))
		c -= 'A'-'a';
	return c;
}

/** Makes the input character upper case.\n Will do nothing if it 
 * was something different than a lower case letter before. */
static inline unsigned char toupper(unsigned char c)
{
	if (islower(c))
		c -= 'a'-'A';
	return c;
}

/*Makes a whole string into lowercase*/
static inline unsigned char* stolower(unsigned char* c)
{
    size_t len = 0;
    while (c[len]) {
        tolower(c[len]);
        len++;
    }
    return c;
}

/*Makes a whole string into uppercase*/
static inline unsigned char* stoupper(unsigned char* c)
{
    size_t len = 0;
    while (c[len]) {
        toupper(c[len]);
        len++;
    }
    return c;
}

#endif
