#ifndef _BUFFERED_READER_H_
#define _BUFFERED_READER_H_

#include <stdio.h>
#include <sys/types.h>

#include "chunk.h"

/**
 * @brief Buffered reader.
 */
struct buffered_reader {
	char			field_delim;
	int			key_field;
	ssize_t			chunk_size;
	FILE *			fp;
	char *			buf;
	size_t			buf_len;
	size_t			off;
	char **			header_lines;
	size_t			nr_header_lines;
};

/**
 * @brief Create a buffered reader.
 * 
 * @param fp			input file
 * @param field_delim		field delimiter
 * @param key_field		key field
 * @param chunk_size		chunk size
 * 
 * @return buffered reader
 */
struct buffered_reader *buffered_reader_create(FILE *fp, char field_delim, int key_field, ssize_t chunk_size);

/**
 * @brief Free a buffered reader.
 * 
 * @param br 			buffered reader
 */
void buffered_reader_free(struct buffered_reader *br);

/**
 * @brief Read header.
 * 
 * @param br 			buffered reader
 * @param header 		number of header lines
 */
void buffered_reader_read_header(struct buffered_reader *br, size_t header);

/**
 * @brief Read next chunk.
 * 
 * @param br 			buffered reader
 *
 * @return next chunk
 */
struct chunk *buffered_reader_read_chunk(struct buffered_reader *br);

#endif