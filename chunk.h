#ifndef _CHUNK_H_
#define _CHUNK_H_

#include "buffered_reader.h"

/**
 * @brief Chunk.
 */
struct chunk {
	FILE *				fp;
	struct line_array *		larr;
	struct buffered_reader *	br;
	size_t				larr_idx;
	struct line 			current_line;
	struct chunk *			next;
};

/**
 * @brief Create a chunk.
 * 
 * @return chunk
 */
struct chunk *chunk_create();

/**
 * @brief Free a chunk.
 * 
 * @param chunk 	chunk
 */
void chunk_free(struct chunk *chunk);

/**
 * @brief Clear a chunk.
 * 
 * @param chunk 		chunk
 */
void chunk_clear(struct chunk *chunk);

/**
 * @brief Sort and write a chunk on disk.
 * 
 * @param chunk 		chunk
 * @param nr_threads		number of threads to use
 *
 * @return status
 */
int chunk_sort_write(struct chunk *chunk, size_t nr_threads);

/**
 * @brief Prepare chunk read.
 * 
 * @param chunk 		chunk
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 * @param chunk_size 		chunk size
 */
void chunk_prepare_read(struct chunk *chunk, char field_delim, int key_field, ssize_t chunk_size);

/**
 * @brief Peek a line from a chunk.
 * 
 * @param chunk 		chunk
 */
void chunk_peek_line(struct chunk *chunk);

/**
 * @brief Get minimum line from a list of chunks.
 * 
 * @param chunks 		chunks
 *
 * @return chunk containing minimum line
 */
struct chunk *chunk_min_line(struct chunk *chunks);

#endif