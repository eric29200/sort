#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "mem.h"

/**
 * @brief Create a chunk.
 * 
 * @return chunk
 */
struct chunk *chunk_create()
{
	struct chunk *chunk;

	chunk = (struct chunk *) xmalloc(sizeof(struct chunk));
	chunk->larr = line_array_create();
	chunk->current_line.value = NULL;
	chunk->current_line.value_len = 0;
	chunk->fp = NULL;
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

	/* free memory */
	chunk_clear(chunk);
	line_array_free(chunk->larr);
	free(chunk);
}

/**
 * @brief Clear a chunk.
 * 
 * @param chunk 		chunk
 */
void chunk_clear(struct chunk *chunk)
{
	if (!chunk)
		return;

	/* clear lines */
	line_array_clear(chunk->larr);

	/* clear current line */
	if (chunk->current_line.value)
		line_free(&chunk->current_line);
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
	size_t i;

	if (!chunk)
		return 0;

	/* write chunk */
	for (i = 0; i < chunk->larr->size; i++) {
		if (fwrite(chunk->larr->lines[i].value, chunk->larr->lines[i].value_len, 1, chunk->fp) != 1) {
			fprintf(stderr, "Can't write chunk\n");
			return -1;
		}
	}

	return 0;
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
 * @brief Peek a line from a chunk.
 * 
 * @param chunk 		chunk
 * @param line			line
 * @param len			line length
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 */
void chunk_peek_line(struct chunk *chunk, char **line, size_t *len, char field_delim, int key_field)
{
	size_t line_len;

	/* get next line */
	if (getline(line, len, chunk->fp) == -1) {
		line_free(&chunk->current_line);
		return;
	}

	/* grow current line if needed */
	line_len = strlen(*line);
	if (line_len > chunk->current_line.value_len)
		chunk->current_line.value = xrealloc(chunk->current_line.value, line_len + 1);

	/* copy line */
	memcpy(chunk->current_line.value, *line, line_len + 1);

	/* init line */
	line_init(&chunk->current_line, chunk->current_line.value, line_len, field_delim, key_field);
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