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
	struct line_array *	line_array;	
	ssize_t 		size;
	struct line 		current_line;
};

/**
 * @brief Create a chunk.
 * 
 * @param fp 		chunk file
 *
 * @return chunk
 */
struct chunk *chunk_create(FILE *fp);

/**
 * @brief Free a chunk.
 * 
 * @param chunk 		chunk
 */
void chunk_free(struct chunk *chunk);

/**
 * @brief Add a line to a chunk.
 * 
 * @param chunk 		chunk
 * @param value 		line value
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 */
void chunk_add_line(struct chunk *chunk, const char *value, char field_delim, int key_field);

/**
 * @brief Peek a line from a chunk.
 * 
 * @param chunk 		chunk
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 */
void chunk_peek_line(struct chunk *chunk, char field_delim, int key_field);

/**
 * @brief Get minimum line from a list of chunks.
 * 
 * @param chunks 		chunks
 * @param nr_chunks 		number of chunks
 *
 * @return chunk index containing minimum line
 */
int chunk_min_line(struct chunk **chunks, size_t nr_chunks);

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