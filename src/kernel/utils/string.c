/**
 * @file string.chr
 * @brief Implementation of minimal string functions.
 */

#include "string.h"

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
void *memcpy(void *dest, const void *src, size_t size)
{
	char *d_ptr = (char *)dest;
	const char *s_ptr = (const char *)src;
	while (size--) {
		*d_ptr++ = *s_ptr++;
	}
	return dest;
}

void *memset(void *s_ptr, int val, size_t size)
{
	unsigned char *ptr = (unsigned char *)s_ptr;
	while (size--) {
		*ptr++ = (unsigned char)val;
	}
	return s_ptr;
}

int memcmp(const void *str1, const void *str2, size_t size)
{
	const unsigned char *ptr1 = (const unsigned char *)str1;
	const unsigned char *ptr2 = (const unsigned char *)str2;
	while (size--) {
		if (*ptr1 != *ptr2) {
			return (int)(*ptr1 - *ptr2);
		}
		ptr1++;
		ptr2++;
	}
	return 0;
}

void *memmove(void *dest, const void *src, size_t size)
{
	char *d_ptr = (char *)dest;
	const char *s_ptr = (const char *)src;

	if (d_ptr < s_ptr) {
		/* Copy forward */
		while (size--) {
			*d_ptr++ = *s_ptr++;
		}
	} else if (d_ptr > s_ptr) {
		/* Copy backward to handle overlap safely */
		d_ptr += size;
		s_ptr += size;
		while (size--) {
			*--d_ptr = *--s_ptr;
		}
	}
	return dest;
}

size_t strlen(const char *str)
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

size_t strnlen(const char *str, size_t maxlen)
{
	size_t len = 0;
	while (len < maxlen && str[len])
		len++;
	return len;
}

void *memchr(const void *str, int chr, size_t size)
{
	const unsigned char *ptr = (const unsigned char *)str;
	while (size--) {
		if (*ptr == (unsigned char)chr)
			return (void *)ptr;
		ptr++;
	}
	return NULL;
}

char *strrchr(const char *str, int chr)
{
	char *last = NULL;
	do {
		if (*str == (char)chr)
			last = (char *)str;
	} while (*str++);
	return last;
}

/**
 * @brief Finds the first occurrence of 'chr' in string 's'.
 */
char *strchr(const char *str, int chr)
{
	while (*str != (char)chr) {
		if (!*str++) {
			return NULL;
		}
	}
	return (char *)str;
}

/**
 * @brief Minimal implementation of strtoul for AArch64 kernel utility.
 * Handles base 10 and base 16 (including optional 0x prefix).
 */
unsigned long strtoul(const char *nptr, char **endptr, int base)
{
	unsigned long res = 0;

	// Auto-detect or handle hex prefix
	if ((base == 0 || base == 16) && nptr[0] == '0' &&
	    (nptr[1] == 'x' || nptr[1] == 'X')) {
		base = 16;
		nptr += 2;
	} else if (base == 0) {
		base = 10;
	}

	while (*nptr) {
		unsigned char chr = *nptr;
		int value;

		if (chr >= '0' && chr <= '9')
			value = chr - '0';
		else if (chr >= 'a' && chr <= 'f')
			value = chr - 'a' + 10;
		else if (chr >= 'A' && chr <= 'F')
			value = chr - 'A' + 10;
		else
			break;

		if (value >= base)
			break;

		res = res * base + value;
		nptr++;
	}

	if (endptr)
		*endptr = (char *)nptr;
	return res;
}

__typeof(strtoul) __isoc23_strtoul __attribute__((alias("strtoul")));

// NOLINTEND(bugprone-easily-swappable-parameters)
