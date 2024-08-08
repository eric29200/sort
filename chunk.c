#include "chunk.h"

/**
 * @brief Create a chunk.
 * 
 * @param fp 		chunk file
 *
 * @return chunk
 */
struct chunk *chunk_create(FILE *fp)
{
	struct chunk *chunk;

	/* allocate a new chunk */
	chunk = xmalloc(sizeof(struct chunk));
	chunk->line_array = line_array_create();
	chunk->size = 0;
	chunk->current_line.value = NULL;

	/* set or create file */
	if (fp) {
		chunk->fp = fp;
	} else {
		/* create temp file if no output file specified */
		chunk->fp = tmpfile();
		if (!chunk->fp) {
			perror("tmpfile");
			xfree(chunk);
			return NULL;
		}
	}

	return chunk;
}

/**
 * @brief Clear a chunk.
 * 
 * @param chunk 		chunk
 */
static void __chunk_clear(struct chunk *chunk)
{
	if (!chunk)
		return;

	/* clear lines */
	line_array_clear_full(chunk->line_array);
	chunk->size = 0;

	/* clear current line */
	if (chunk->current_line.value)
		line_free(&chunk->current_line);
}

/**
 * @brief Free a chunk.
 * 
 * @param chunk 		chunk
 */
void chunk_free(struct chunk *chunk)
{
	if (chunk) {
		fclose(chunk->fp);
		__chunk_clear(chunk);
		line_array_free_full(chunk->line_array);
		free(chunk);
	}
}


/**
 * @brief Add a line to a chunk.
 * 
 * @param chunk 		chunk
 * @param value 		line value
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 */
void chunk_add_line(struct chunk *chunk, const char *value, char field_delim, int key_field)
{
	size_t value_len;

	if (!chunk || !value)
		return;

	value_len = strlen(value);
	line_array_add(chunk->line_array, xstrdup(value), value_len, field_delim, key_field);
	chunk->size += value_len;
}

/**
 * @brief Peek a line from a chunk.
 * 
 * @param chunk 		chunk
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 */
void chunk_peek_line(struct chunk *chunk, char field_delim, int key_field)
{
	char *line = NULL;
	size_t len;

	/* free previous line */
	line_free(&chunk->current_line);

	/* get next line */
	if (getline(&line, &len, chunk->fp) != -1) {
		line_init(&chunk->current_line, xstrdup(line), strlen(line), field_delim, key_field);
		free(line);
	}
}

/**
 * @brief Get minimum line from a list of chunks.
 * 
 * @param chunks 		chunks
 * @param nr_chunks 		number of chunks
 *
 * @return chunk index containing minimum line
 */
int chunk_min_line(struct chunk **chunks, size_t nr_chunks)
{
	int min = -1;
	size_t i;

	for (i = 0; i < nr_chunks; i++) {
		if (!chunks[i]->current_line.value)
			continue;

		if (min == -1 || line_compare(&chunks[i]->current_line, &chunks[min]->current_line) < 0)
			min = i;
	}

	return min;
}

/**
 * @brief Write a chunk on disk.
 * 
 * @param chunk 		chunk
 *
 * @return status
 */
int chunk_write(struct chunk *chunk)
{
	size_t i;

	if (!chunk)
		return 0;

	/* write chunk */
	for (i = 0; i < chunk->line_array->size; i++) {
		if (!fputs(chunk->line_array->lines[i].value, chunk->fp)) {
			perror("fputs");
			return -1;
		}
	}

	/* clear chunk */
	__chunk_clear(chunk);

	return 0;
}

/**
 * @brief Sort and write a chunk on disk.
 * 
 * @param chunk 		chunk
 * @param nr_threads		number of threads to use
 *
 * @return status
 */
int chunk_sort_write(struct chunk *chunk, size_t nr_threads)
{
	/* sort chunk */
	line_array_sort(chunk->line_array, nr_threads);

	/* write chunk */
	return chunk_write(chunk);
}