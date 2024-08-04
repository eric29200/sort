#include <stdlib.h>
#include <string.h>

#include "tsort.h"
#include "mem.h"

#define BUCKET_SIZE		32
#define MIN(x, y)		((x) < (y) ? (x) : (y))

/**
 * @brief Swap 2 items.
 * 
 * @param i1 		first item
 * @param i2 		second item
 * @param tmp 		tmp item
 * @param item_size 	item size
 */
static inline void __swap(void *i1, void *i2, void *tmp, size_t item_size)
{
	if (i1 != i2) {
		memcpy(tmp, i1, item_size);
		memcpy(i1, i2, item_size);
		memcpy(i2, tmp, item_size);
	}
}

/**
 * @brief Insertion sort.
 * 
 * @param data 		data
 * @param item_size	item size 
 * @param left 		left index
 * @param right 	right index
 * @param tmp 		tmp item
 * @param compare 	compare function
 */
static void __insertion_sort(void *data, size_t item_size, int left, int right, void *tmp, int (*compare)(const void *, const void *))
{
	int i, j;

	for (i = left + 1; i <= right; i++) {
		memcpy(tmp, data + i * item_size, item_size);

		for (j = i - 1; j >= left && compare(data + j * item_size, tmp) > 0; j--)
			memcpy(data + (j + 1) * item_size, data + j * item_size, item_size);

		memcpy(data + (j + 1) * item_size, tmp, item_size);
	}
}

/**
 * @brief Merge two sorted sub arrays.
 * 
 * @param data 		data
 * @param item_size 	item size
 * @param left		left index (start of first sub array)
 * @param mid 		mid (end of first array, start of second sub array)
 * @param right 	right index (end of second sub array)
 * @param tmp 		tmp item
 * @param compare 	compare function
 */
static void __merge(void *data, size_t item_size, int left, int mid, int right, void *tmp, int (*compare)(const void *, const void *))
{
	int len1, len2, i, j, k;

	/* compute first sub array length */
	len1 = mid - left + 1;

	/* compute second sub array length */
	len2 = right - mid;

	/* merge sub arrays */
	for (i = 0, j = 0, k = 0; i < len1 && j < len2; k++) {
		if (compare(data + (left + i) * item_size, data + (mid + 1 + j) * item_size) <= 0)
			__swap(data + k * item_size, data + (left + i++) * item_size, tmp, item_size);
		else
			__swap(data + k * item_size, data + (mid + 1 + j++) * item_size, tmp, item_size);
	}

	/* copy remaning items of left sub array */
	for (; i < len1; i++, k++)
		__swap(data + k * item_size, data + (left + i) * item_size, tmp, item_size);

	/* copy remaning items of right sub array */
	for (; j < len2; j++, k++)
		__swap(data + k * item_size, data + (mid + 1 + j) * item_size, tmp, item_size);
}

/**
 * @brief Tim sort.
 * 
 * @param data 		data
 * @param size 		number of items
 * @param item_size 	item size
 * @param compare 	compare function
 */
void tim_sort(void *data, int size, size_t item_size, int (*compare)(const void *, const void *))
{
	int left, mid, right, len, i;
	void *tmp;

	/* check input array */
	if (!data || size < 2)
		return;

	/* allocate tmp */
	tmp = xmalloc(item_size);

	/* sort buckets */
	for (i = 0; i < size; i += BUCKET_SIZE)
		__insertion_sort(data, item_size, i, MIN(i + BUCKET_SIZE - 1, size - 1), tmp, compare);

	/* merge buckets */
	for (len = BUCKET_SIZE; len < size; len = 2 * len) {
		for (left = 0; left < size; left += 2 * len) {
			mid = left + len - 1;
			right = MIN(left + 2 * len - 1, size - 1);

			if (mid < right)
				__merge(data, item_size, left, mid, right, tmp, compare);
		}
	}

	/* free tmp */
	xfree(tmp);
}