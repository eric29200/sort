#ifndef _CHUNK_H_
#define _CHUNK_H_

#include <stdlib.h>
#include <string.h>

#include "line.h"
#include "mem.h"

/**
 * @brief Chunk structure.
 */
struct chunk {
	FILE *			fp;
	char			close_on_free;
	struct line_array *	line_array;	
	struct line 		current_line;
	struct chunk *		next;
};

/**
 * @brief Create a chunk.
 * 
 * @param fp 		chunk file
 * @param close_on_free	close file on free ?
 *
 * @return chunk
 */
struct chunk *chunk_create(FILE *fp, char close_on_free);

/**
 * @brief Free a chunk.
 * 
 * @param chunk 		chunk
 */
void chunk_free(struct chunk *chunk);

/**
 * @brief Clear a chunk.
 * 
 * @param chunk 		chunk
 */
void chunk_clear(struct chunk *chunk);

/**
 * @brief Add a line to a chunk.
 * 
 * @param chunk 		chunk
 * @param value 		line value
 * @param value_len		line value length
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 */
void chunk_add_line(struct chunk *chunk, char *value, size_t value_len, char field_delim, int key_field);

/**
 * @brief Peek a line from a chunk.
 * 
 * @param chunk 		chunk
 * @param line			line
 * @param len			line length
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 */
void chunk_peek_line(struct chunk *chunk, char **line, size_t *len, char field_delim, int key_field);

/**
 * @brief Get minimum line from a list of chunks.
 * 
 * @param chunks 		chunks
 *
 * @return chunk containing minimum line
 */
struct chunk *chunk_min_line(struct chunk *chunks);

/**
 * @brief Write a chunk on disk.
 * 
 * @param chunk 		chunk
 *
 * @return status
 */
int chunk_write(struct chunk *chunk);

/**
 * @brief Sort and write a chunk on disk.
 * 
 * @param chunk 		chunk
 * @param nr_threads		number of threads to use
 *
 * @return status
 */
int chunk_sort_write(struct chunk *chunk, size_t nr_threads);

#endif