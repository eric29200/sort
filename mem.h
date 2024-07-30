#ifndef _MEM_H_
#define _MEM_H_

#include <stdio.h>

/**
 * @brief Malloc or exit.
 * 
 * @param size 		size to allocate
 *
 * @return allocated memory
 */
void *xmalloc(size_t size);

/**
 * @brief Realloc or exit.
 * 
 * @param ptr		memory to reallocate
 * @param size 		size to allocate
 *
 * @return allocated memory
 */
void *xrealloc(void *ptr, size_t size);

/**
 * @brief Safe free.
 * 
 * @param ptr 		memory to free
 */
void xfree(void *ptr);

/**
 * @brief Duplicate a string or exit.
 * 
 * @param s 		string to duplicate
 *
 * @return duplicated string
 */
void *xstrdup(const char *s);

#endif