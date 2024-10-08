#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "line.h"
#include "mem.h"

#define NR_BUCKETS			256
#define INITIAL_SIZE			10

/**
 * @brief Thread sort argument.
 */
struct thread_sort_arg {
	struct line_array **	buckets;
	size_t			i;
	pthread_mutex_t 	lock;
};


/**
 * @brief Init a line.
 * 
 * @param line			line
 * @param value 		line value
 * @param value_len		value length
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 */
void line_init(struct line *line, char *value, int value_len, char field_delim, int key_field)
{
	char *kend;

	/* set value */
	line->value = value;
	line->value_len = value_len;

	/* find key start */
	line->key = line->value;
	while (key_field-- && (line->key = strchr(line->key, field_delim)))
		line->key++;

	/* key out of value */
	if (line->key >= value + value_len)
		line->key = NULL;

	/* compute key end and length */
	if (line->key) {
		kend = strchrnul(line->key, field_delim);
		if (kend > value + value_len)
			kend = value + value_len;

		line->key_len = (size_t) (kend - line->key);
	} else {
		line->key_len = 0;
	}
}

/**
 * @brief Compare 2 lines.
 * 
 * @param line1 	first line
 * @param line2 	second line
 *
 * @return comparison result
 */
int line_compare(const struct line *line1, const struct line *line2)
{
	size_t len;
	int ret;

	/* find maximum length */
	len = line1->key_len < line2->key_len ? line1->key_len : line2->key_len;

	/* compare keys */
	ret = strncmp(line1->key, line2->key, len);
	if (ret)
		return ret;

	return line1->key_len - line2->key_len;
}

/**
 * @brief Create a line array.
 * 
 * @param capacity	initial capacity
 * @param grow_slow	grow slow ?
 * 
 * @return line array
 */
struct line_array *line_array_create(size_t capacity, char grow_slow)
{
	struct line_array *larr;

	/* create array */
	larr = (struct line_array *) xmalloc(sizeof(struct line_array));
	larr->capacity = capacity;
	larr->size = 0;
	larr->grow_slow = grow_slow;

	/* allocate array */
	if (capacity)
		larr->lines = (struct line *) xmalloc(sizeof(struct line) * capacity);
	else
		larr->lines = NULL;

	return larr;
}

/**
 * @brief Free a line array.
 * 
 * @param larr 		line array
 */
void line_array_free(struct line_array *larr)
{
	if (!larr)
		return;

	line_array_clear_full(larr);
	free(larr);
}

/**
 * @brief Clear a line array.
 * 
 * @param larr 		line array
 */
void line_array_clear_full(struct line_array *larr)
{
	if (!larr)
		return;

	/* clear lines */
	if (larr->lines) {
		free(larr->lines);
		larr->lines = NULL;
	}

	/* reset size */
	larr->size = 0;
	larr->capacity = 0;
}

/**
 * @brief Grow a line array.
 * 
 * @param larr 		line array
 */
static void __line_array_grow(struct line_array *larr)
{
	/* no need to grow */
	if (larr->size != larr->capacity)
		return;

	/* set new capacity */
	if (larr->grow_slow) {
		larr->capacity++;
	} else {
		larr->capacity = larr->capacity + (larr->capacity >> 1);
		if (larr->capacity < INITIAL_SIZE)
			larr->capacity = INITIAL_SIZE;
	}
	
	/* reallocate lines */
	larr->lines = (struct line *) xrealloc(larr->lines, sizeof(struct line) * larr->capacity);
}

/**
 * @brief Add a line.
 * 
 * @param larr			line array
 * @param value 		line value
 * @param value_len		line value length
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 */
void line_array_add(struct line_array *larr, char *value, size_t value_len, char field_delim, int key_field)
{
	/* grow lines array if needed */
	__line_array_grow(larr);

	/* add line */
	line_init(&larr->lines[larr->size++], value, value_len, field_delim, key_field);
}

/**
 * @brief Add a line.
 * 
 * @param larr			line array
 * @param line			line to add
 */
static void __line_array_add(struct line_array *larr, struct line *line)
{
	/* grow lines array if needed */
	__line_array_grow(larr);

	/* add line */
	larr->lines[larr->size++] = *line;
}

/**
 * @brief Sort a line array.
 * 
 * @param lines 		lines
 * @param nr_lines		number of lines
 */
static void __qsort(struct line *lines, size_t nr_lines)
{
	struct line pivot, tmp;
	int i, j;

	if (nr_lines < 2)
		return;

	pivot = lines[nr_lines / 2];

	for (i = 0, j = nr_lines - 1; ; i++, j--) {
		while (line_compare(&lines[i], &pivot) < 0)
			i++;

		while (line_compare(&lines[j], &pivot) > 0)
			j--;

		if (i >= j)
			break;

		tmp = lines[i];
		lines[i] = lines[j];
		lines[j] = tmp;
	}

	__qsort(lines, i);
	__qsort(lines + i, nr_lines - i);
}

/**
 * @brief Sort a line array (thread function).
 * 
 * @param arg 			thread argument
 *
 * @return status
 */
static void *__sort_thread(void *arg)
{
	struct thread_sort_arg *targ = (struct thread_sort_arg *) arg;
	struct line_array *larr;

	for (;;) {
		/* get next bucket */
		pthread_mutex_lock(&targ->lock);
		for (;;) {
			/* no more buckets */
			if (targ->i >= NR_BUCKETS) {
				pthread_mutex_unlock(&targ->lock);
				goto end;
			}

			/* get next bucket */
			larr = targ->buckets[targ->i++];
			if (larr)
				break;
		}
		pthread_mutex_unlock(&targ->lock);
	
		/* sort bucket */
		__qsort(larr->lines, larr->size);
	}

end:
	return NULL;
}

/**
 * @brief Create buckets.
 * 
 * @param larr 		line array
 *
 * @return buckets
 */
static struct line_array **__create_buckets(struct line_array *larr)
{
	int counts[NR_BUCKETS] = { 0 };
	struct line_array **buckets;
	size_t i;

	/* compute sizes of buckets */
	memset(counts, 0, sizeof(int) * NR_BUCKETS);
	for (i = 0; i < larr->size; i++)
		counts[(unsigned char) larr->lines[i].key[0]]++;

	/* create buckets */
	buckets = (struct line_array **) xmalloc(NR_BUCKETS * sizeof(struct line_array *));
	for (i = 0; i < NR_BUCKETS; i++)
		buckets[i] = counts[i] > 0 ? line_array_create(counts[i], 1) : NULL;

	/* populate buckets */
	for (i = 0; i < larr->size; i++)
		__line_array_add(buckets[(unsigned char) larr->lines[i].key[0]], &larr->lines[i]);

	return buckets;
}

/**
 * @brief Sort a line array.
 * 
 * @param larr		line array
 * @param nr_threads	number of threads to use
 */
void line_array_sort(struct line_array *larr, size_t nr_threads)
{
	pthread_t threads[nr_threads < 1 ? 1 : nr_threads];
	struct thread_sort_arg targ;
	size_t i, j, k;

	/* fix number of threads */
	if (nr_threads < 1)
		nr_threads = 1;
	
	/* init threads arguments */
	targ.buckets = __create_buckets(larr);
	targ.i = 0;
	pthread_mutex_init(&targ.lock, NULL);

	/* create threads */
	for (i = 0; i < nr_threads; i++)
		pthread_create(&threads[i], NULL, __sort_thread, &targ);
	
	/* wait for threads */
	for (i = 0; i < nr_threads; i++)
		pthread_join(threads[i], NULL);
	
	/* merge buckets */
	for (i = 0, k = 0; i < NR_BUCKETS; i++)
		if (targ.buckets[i])
			for (j = 0; j < targ.buckets[i]->size; j++)
				larr->lines[k++] = targ.buckets[i]->lines[j];

	/* free buckets */
	for (i = 0; i < NR_BUCKETS; i++)
		if (targ.buckets[i])
			line_array_free(targ.buckets[i]);
	xfree(targ.buckets);
	pthread_mutex_destroy(&targ.lock);
}

/**
 * @brief Write a line array on disk.
 * 
 * @param larr			line array
 * @param fp			output file
 *
 * @return status
 */
int line_array_write(struct line_array *larr, FILE *fp)
{
	size_t i;

	/* write chunk */
	for (i = 0; i < larr->size; i++) {
		if (fwrite(larr->lines[i].value, larr->lines[i].value_len, 1, fp) != 1) {
			fprintf(stderr, "Can't write line array\n");
			return -1;
		}
	}

	return 0;
}