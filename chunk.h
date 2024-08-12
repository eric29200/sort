#ifndef _CHUNK_H_
#define _CHUNK_H_

#include "line.h"

/**
 * @brief Chunk.
 */
struct chunk {
	struct line_array *	larr;
	FILE *			fp;
	struct line 		current_line;
	struct chunk *		next;
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

#endif