#include <string.h>
#include <stdlib.h>

#include "line.h"
#include "qsort.h"
#include "mem.h"

/**
 * @brief Init a line.
 * 
 * @param line			line
 * @param value 		line value
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 */
void line_init(struct line *line, char *value, char field_delim, int key_field)
{
	char *kend;

	/* set value */
	line->value = value;

	/* find key start */
	line->key = line->value;
	while (key_field-- && (line->key = strchr(line->key, field_delim)))
		line->key++;

	/* compute key end and length */
	if (line->key) {
		kend = strchr(line->key, field_delim);
		line->key_len = kend ? kend - line->key : strlen(line->key);
	} else {
		line->key_len = 0;
	}
}

/**
 * @brief Free a line.
 * 
 * @param line 		line
 */
void line_free(struct line *line)
{
	free(line->value);
	line->value = NULL;
}

/**
 * @brief Compare 2 lines.
 * 
 * @param l1 		first line
 * @param l2 		second line
 *
 * @return comparison result
 */
int line_compare(const void *l1, const void *l2)
{
	struct line *line1 = (struct line *) l1;
	struct line *line2 = (struct line *) l2;
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
 * @return line array
 */
struct line_array *line_array_create()
{
	struct line_array *larr;

	larr = (struct line_array *) xmalloc(sizeof(struct line_array));
	larr->lines = NULL;
	larr->size = 0;
	larr->capacity = 0;

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

	line_array_clear(larr);
	free(larr);
}

/**
 * @brief Free a line array.
 * 
 * @param larr 		line array
 */
void line_array_free_full(struct line_array *larr)
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
void line_array_clear(struct line_array *larr)
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
 * @brief Clear a line array.
 * 
 * @param larr 		line array
 */
void line_array_clear_full(struct line_array *larr)
{
	size_t i;

	if (!larr)
		return;

	/* clear lines */
	if (larr->lines) {
		for (i = 0; i < larr->size; i++)
			line_free(&larr->lines[i]);

		free(larr->lines);
		larr->lines = NULL;
	}

	/* reset size */
	larr->size = 0;
	larr->capacity = 0;
}

/**
 * @brief Add a line.
 * 
 * @param larr			line array
 * @param value 		line value
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 */
void line_array_add(struct line_array *larr, char *value, char field_delim, int key_field)
{
	if (!larr || !value)
		return;

	/* grow lines array if needed */
	if (larr->size == larr->capacity) {
		larr->capacity = larr->capacity + (larr->capacity >> 1);
		if (larr->capacity < 10)
			larr->capacity = 10;
		larr->lines = (struct line *) xrealloc(larr->lines, sizeof(struct line) * larr->capacity);
	}

	/* add line */
	line_init(&larr->lines[larr->size], value, field_delim, key_field);
	larr->size++;
}

/**
 * @brief Sort a line array.
 * 
 * @param larr		line array
 */
void line_array_sort(struct line_array *larr)
{
	if (larr->size > 0)
		quick_sort(larr->lines, larr->size, sizeof(struct line), line_compare);
}