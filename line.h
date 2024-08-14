#ifndef _LINE_H_
#define _LINE_H_

#include <stdio.h>

/**
 * @brief Line structure.
 */
struct line {
	char *			value;
	int			value_len;
	char *			key;
	int 			key_len;
} __attribute__((packed));

/**
 * @brief Line array structure.
 * 
 */
struct line_array {
	struct line *		lines;
	size_t			size;
	size_t			capacity;
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
void line_init(struct line *line, char *value, int value_len, char field_delim, int key_field);

/**
 * @brief Compare 2 lines.
 * 
 * @param l1 		first line
 * @param l2 		second line
 *
 * @return comparison result
 */
int line_compare(const void *l1, const void *l2);

/**
 * @brief Create a line array.
 * 
 * @return line array
 */
struct line_array *line_array_create();

/**
 * @brief Free a line array.
 * 
 * @param larr 		line array
 */
void line_array_free(struct line_array *larr);

/**
 * @brief Clear a line array.
 * 
 * @param larr 		line array
 */
void line_array_clear(struct line_array *larr);

/**
 * @brief Add a line.
 * 
 * @param larr		line array
 * @param value 	line value
 * @param value_len	line value length
 * @param field_delim	field delimiter
 * @param key_field 	key field
 */
void line_array_add(struct line_array *larr, char *value, size_t value_len, char field_delim, int key_field);

/**
 * @brief Sort a line array.
 * 
 * @param larr		line array
 * @param nr_threads	number of threads to use
 */
void line_array_sort(struct line_array *larr, size_t nr_threads);

/**
 * @brief Write a line array on disk.
 * 
 * @param larr			line array
 * @param fp			output file
 *
 * @return status
 */
int line_array_write(struct line_array *larr, FILE *fp);

#endif