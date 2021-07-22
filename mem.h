#ifndef _MEM_H_
#define _MEM_H_

#include <stdio.h>

void *xmalloc(size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xrealloc(void *ptr, size_t size);
void xfree(void *ptr);
void *xstrdup(const char *s);

#endif
