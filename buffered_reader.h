#ifndef _BUFFERED_READER_H_
#define _BUFFERED_READER_H_

#include <stdio.h>
#include <sys/types.h>

#include "line.h"

/**
 * @brief Buffered reader.
 */
struct buffered_reader {
	char			field_delim;
	int			key_field;
	FILE *			fp;
	char *			buf;
	size_t			buf_len;
	size_t			buf_capacity;
	size_t			off;
	char **			header_lines;
	size_t			nr_header_lines;
	size_t			line_len;
};

/**
 * @brief Create a buffered reader.
 * 
 * @param fp			input file
 * @param field_delim		field delimiter
 * @param key_field		key field
 * @param header		number of header lines
 * @param memory_size		memory size
 * 
 * @return buffered reader
 */
struct buffered_reader *buffered_reader_create(FILE *fp, char field_delim, int key_field, size_t header, ssize_t memory_size);

/**
 * @brief Free a buffered reader.
 * 
 * @param br 			buffered reader
 */
void buffered_reader_free(struct buffered_reader *br);

/**
 * @brief Read next lines.
 * 
 * @param br 			buffered reader
 * @param larr			lines array
 */
void buffered_reader_read_lines(struct buffered_reader *br, struct line_array *larr);

#endif