#include "chunk.h"

/**
 * @brief Create a chunk.
 * 
 * @param fp 		chunk file
 * @param close_on_free	close file on free ?
 *
 * @return chunk
 */
struct chunk *chunk_create(FILE *fp, char close_on_free)
{
	struct chunk *chunk;

	/* allocate a new chunk */
	chunk = xmalloc(sizeof(struct chunk));
	chunk->line_array = line_array_create();
	chunk->size = 0;
	chunk->current_line.value = NULL;
	chunk->current_line.value_len = 0;
	chunk->close_on_free = close_on_free;
	chunk->next = NULL;

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
	if (!chunk)
		return;

	/* close file */
	if (chunk->close_on_free && chunk->fp)
		fclose(chunk->fp);
		
	/* clear memory */
	__chunk_clear(chunk);
	line_array_free_full(chunk->line_array);
	free(chunk);
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
		if (fwrite(chunk->line_array->lines[i].value, chunk->line_array->lines[i].value_len, 1, chunk->fp) != 1) {
			fprintf(stderr, "Can't write chunk\n");
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