#ifndef _QSORT_H_
#define _QSORT_H_

#include <stdio.h>

/**
 * @brief Quick sort.
 * 
 * @param data 		data
 * @param size 		number of items
 * @param item_size 	item size
 * @param compare 	compare function
 */
void quick_sort(void *data, int size, size_t item_size, int (*compare)(const void *, const void *));

#endif