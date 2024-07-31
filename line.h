#ifndef _LINE_H_
#define _LINE_H_

#include <stdio.h>

/**
 * @brief Line structure.
 */
struct line {
	char *			value;
	char *			key;
	size_t 			key_len;
};

/**
 * @brief Init a line.
 * 
 * @param line			line
 * @param value 		line value
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 */
void line_init(struct line *line, const char *value, char field_delim, int key_field);

/**
 * @brief Free a line.
 * 
 * @param line 		line
 */
void line_free(struct line *line);

/**
 * @brief Compare 2 lines.
 * 
 * @param l1 		first line
 * @param l2 		second line
 *
 * @return comparison result
 */
int line_compare(const void *l1, const void *l2);

#endif