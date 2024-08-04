#ifndef _TSORT_H_
#define _TSORT_H_

#include <stdio.h>

/**
 * @brief Tim sort.
 * 
 * @param data 		data
 * @param size 		number of items
 * @param item_size 	item size
 * @param compare 	compare function
 */
void tim_sort(void *data, int size, size_t item_size, int (*compare)(const void *, const void *));

#endif