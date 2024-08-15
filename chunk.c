#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "mem.h"

/**
 * @brief Create a chunk.
 * 
 * @param capacity	capacity
 * 
 * @return chunk
 */
struct chunk *chunk_create(size_t capacity)
{
	struct chunk *chunk;

	chunk = (struct chunk *) xmalloc(sizeof(struct chunk));
	chunk->larr = line_array_create(capacity, 1);
	chunk->current_line.value = NULL;
	chunk->current_line.value_len = 0;
	chunk->fp = NULL;
	chunk->br = NULL;
	chunk->larr_idx = 0;
	chunk->next = NULL;

	return chunk;
}

/**
 * @brief Free a chunk.
 * 
 * @param chunk 	chunk
 */
void chunk_free(struct chunk *chunk)
{
	if (!chunk)
		return;

	/* close file */
	if (chunk->fp)
		fclose(chunk->fp);

	/* free buffered reader */
	if (chunk->br)
		buffered_reader_free(chunk->br);

	/* free memory */
	chunk_clear_full(chunk);
	line_array_free(chunk->larr);
	free(chunk);
}

/**
 * @brief Clear a chunk (free lines array).
 * 
 * @param chunk 		chunk
 */
void chunk_clear_full(struct chunk *chunk)
{
	line_array_clear_full(chunk->larr);
	chunk->larr_idx = 0;
}

/**
 * @brief Write a chunk on disk.
 * 
 * @param chunk 		chunk
 *
 * @return status
 */
static int __chunk_write(struct chunk *chunk)
{
	/* create temp file */
	chunk->fp = tmpfile();
	if (!chunk->fp) {
		fprintf(stderr, "Can't create temporary file\n");
		return -1;
	}

	/* write lines */
	return line_array_write(chunk->larr, chunk->fp);
}

/**
 * @brief Sort and write a chunk on disk.
 * 
 * @param chunk 		chunk
 * @param fp			output file
 * @param nr_threads		number of threads to use
 *
 * @return status
 */
int chunk_sort_write(struct chunk *chunk, size_t nr_threads)
{
	/* sort chunk */
	line_array_sort(chunk->larr, nr_threads);

	/* write chunk */
	return __chunk_write(chunk);
}

/**
 * @brief Prepare chunk read.
 * 
 * @param chunk 		chunk
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 * @param memory_size		memory size
 */
void chunk_prepare_read(struct chunk *chunk, char field_delim, int key_field, ssize_t memory_size)
{
	/* rewind */
	rewind(chunk->fp);

	/* create buffered reader */
	chunk->br = buffered_reader_create(chunk->fp, field_delim, key_field, 0, memory_size);

	/* clear chunk */
	chunk_clear_full(chunk);

	/* allocate lines array */
	chunk->larr->capacity = memory_size / chunk->br->line_len;
	chunk->larr->lines = (struct line *) xmalloc(sizeof(struct line) * chunk->larr->capacity);

	/* peek first line */
	chunk_peek_line(chunk);
}

/**
 * @brief Peek a line from a chunk.
 * 
 * @param chunk 		chunk
 */
void chunk_peek_line(struct chunk *chunk)
{
	/* read next lines */
	if (chunk->larr_idx == chunk->larr->size) {
		/* reset line array */
		chunk->larr->size = 0;
		chunk->larr_idx = 0;

		/* read next lines */
		buffered_reader_read_lines(chunk->br, chunk->larr);

		/* no more lines */
		if (chunk->larr->size == 0) {
			chunk->current_line.value = NULL;
			return;
		}
	}

	/* peek line */
	memcpy(&chunk->current_line, &chunk->larr->lines[chunk->larr_idx++], sizeof(struct line));
}

/**
 * @brief Get minimum line from a list of chunks.
 * 
 * @param chunks 		chunks
 *
 * @return chunk containing minimum line
 */
struct chunk *chunk_min_line(struct chunk *chunks)
{
	struct chunk *chunk, *min = NULL;

	for (chunk = chunks; chunk != NULL; chunk = chunk->next) {
		if (!chunk->current_line.value)
			continue;

		if (!min || line_compare(&chunk->current_line, &min->current_line) < 0)
			min = chunk;
	}

	return min;
}