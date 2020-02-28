#include <stdlib.h>
#include <string.h>
#include <err.h>

void *sort_malloc(size_t size)
{
	void *ptr;

	ptr = malloc(size);
	if (!ptr)
		err(2, NULL);

	return ptr;
}

void *sort_calloc(size_t nmemb, size_t size)
{
	void *ptr;

	ptr = calloc(nmemb, size);
	if (!ptr)
		err(2, NULL);

	return ptr;
}

void *sort_realloc(void *ptr, size_t size)
{
	ptr = realloc(ptr, size);
	if (!ptr)
		err(2, NULL);

	return ptr;
}

void sort_free(void *ptr)
{
	if (ptr)
		free(ptr);
}

void *sort_strdup(const char *s)
{
	char *dup;

	dup = strdup(s);
	if (!dup)
		err(2, NULL);

	return dup;
}
