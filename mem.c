#include <stdlib.h>
#include <string.h>
#include <err.h>

/*
 * Malloc or exit.
 */
void *xmalloc(size_t size)
{
  void *ptr;

  ptr = malloc(size);
  if (!ptr)
    err(2, NULL);

  return ptr;
}

/*
 * Calloc or exit.
 */
void *xcalloc(size_t nmemb, size_t size)
{
  void *ptr;

  ptr = calloc(nmemb, size);
  if (!ptr)
    err(2, NULL);

  return ptr;
}

/*
 * Realloc or exit.
 */
void *xrealloc(void *ptr, size_t size)
{
  ptr = realloc(ptr, size);
  if (!ptr)
    err(2, NULL);

  return ptr;
}

/*
 * Free memory.
 */
void xfree(void *ptr)
{
  if (ptr)
    free(ptr);
}

/*
 * Strdup or exit.
 */
void *xstrdup(const char *s)
{
  char *dup;

  dup = strdup(s);
  if (!dup)
    err(2, NULL);

  return dup;
}
