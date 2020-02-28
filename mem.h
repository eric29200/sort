#ifndef _MEM_H_
#define _MEM_H_

void *sort_malloc(size_t size);
void *sort_calloc(size_t nmemb, size_t size);
void *sort_realloc(void *ptr, size_t size);
void sort_free(void *ptr);
void *sort_strdup(const char *s);

#endif
