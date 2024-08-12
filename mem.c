#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "mem.h"

/**
 * @brief Malloc or exit.
 * 
 * @param size 		size to allocate
 *
 * @return allocated memory
 */
void *xmalloc(size_t size)
{
	void *ptr;

	ptr = malloc(size);
	if (!ptr)
		err(2, NULL);

	return ptr;
}

/**
 * @brief Realloc or exit.
 * 
 * @param ptr		memory to reallocate
 * @param size 		size to allocate
 *
 * @return allocated memory
 */
void *xrealloc(void *ptr, size_t size)
{
	ptr = realloc(ptr, size);
	if (!ptr)
		err(2, NULL);

	return ptr;
}

/**
 * @brief Safe free.
 * 
 * @param ptr 		memory to free
 */
void xfree(void *ptr)
{
	if (ptr)
		free(ptr);
}

/**
 * @brief Strdup or exit.
 * 
 * @param s 		string to duplicate
 *
 * @return duplicated string
 */
char *xstrdup(const char *s)
{
	char *r = strdup(s);
	if (!r)
		err(2, NULL);

	return r;
}