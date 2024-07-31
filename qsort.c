#include <stdlib.h>
#include <string.h>

#include "qsort.h"
#include "mem.h"

/**
 * @brief Swap 2 items.
 * 
 * @param i1 		first item
 * @param i2 		second item
 * @param tmp 		tmp item
 * @param item_size 	item size
 */
static inline void swap(void *i1, void *i2, void *tmp, size_t item_size)
{
	memcpy(tmp, i1, item_size);
	memcpy(i1, i2, item_size);
	memcpy(i2, tmp, item_size);
}

/**
 * @brief Recursive quick sort.
 * 
 * @param data 		data
 * @param size 		number of items
 * @param item_size 	item size
 * @param tmp 		tmp item
 * @param compare 	compare function
 */
static void __quick_sort(void *data, int size, size_t item_size, void *tmp, int (*compare)(const void *, const void *))
{
	void *pivot;
	int i, j;

	if (size < 2)
		return;

	/* choose pivot */
	pivot = data + (size / 2) * item_size;

	/* sort */
	for (i = 0, j = size - 1;; i++, j--) {
		/* find misplaced item in left partition */
		while (compare(data + i * item_size, pivot) < 0)
			i++;

		/* find misplaced item in right partition */
		while (compare(data + j * item_size, pivot) > 0)
			j--;

		if (i >= j)
			break;

		/* swap items */
		swap(data + i * item_size, data + j * item_size, tmp, item_size);
	}

	/* quicksort remaining */
	__quick_sort(data, i, item_size, tmp, compare);
	__quick_sort(data + i * item_size, size - i, item_size, tmp, compare);
}

/**
 * @brief Quick sort.
 * 
 * @param data 		data
 * @param size 		number of items
 * @param item_size 	item size
 * @param compare 	compare function
 */
void quick_sort(void *data, int size, size_t item_size, int (*compare)(const void *, const void *))
{
	void *tmp;

	/* check input array */
	if (!data || size == 0 || item_size < 2)
		return;

	/* allocate tmp */
	tmp = xmalloc(item_size);

	/* quick sort */
	__quick_sort(data, size, item_size, tmp, compare);

	/* free tmp */
	free(tmp);
}