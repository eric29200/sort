#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "mem.h"
#include "chunk.h"

struct chunk_t *chunk_create(FILE *fp)
{
	struct chunk_t *chunk;

	chunk = sort_malloc(sizeof(struct chunk_t));
	chunk->lines = NULL;
	chunk->nb_lines = 0;
	chunk->size = 0;
	chunk->current_line = NULL;

	if (fp) {
		chunk->fp = fp;
	} else {
		/* create temp file if no output file specified */
		chunk->fp = tmpfile();
		if (!chunk->fp) {
			perror("tmpfile");
			sort_free(chunk);
			return NULL;
		}
	}

	return chunk;
}

void chunk_add_line(struct chunk_t *chunk, const char *value,
		   char field_delim, int key_field)
{
	struct line_t *new_line;

	if (!chunk || !value)
		return;

	new_line = line_create(value, field_delim, key_field);
	chunk->lines = (struct line_t **) sort_realloc(chunk->lines,
						  sizeof(struct line_t *)
						  * (chunk->nb_lines + 1));
	chunk->lines[chunk->nb_lines] = new_line;
	chunk->nb_lines += 1;
	chunk->size += sizeof(new_line) + strlen(new_line->value);
}

void chunk_peek_line(struct chunk_t *chunk, char field_delim, int key_field) {
	char line[LINE_SIZE];

	line_destroy(chunk->current_line);

	if (fgets(line, LINE_SIZE, chunk->fp))
		chunk->current_line = line_create(line, field_delim, key_field);
	else
		chunk->current_line = NULL;
}

void chunk_sort(struct chunk_t *chunk)
{
	if (chunk && chunk->nb_lines > 0)
		qsort(chunk->lines, chunk->nb_lines, sizeof(struct line_t *),
		      line_compare);
}

int chunk_min_line(struct chunk_t **chunks, size_t nb_chunks)
{
	size_t i;
	int min = -1;

	for (i = 0; i < nb_chunks; i++) {
		if (!chunks[i]->current_line)
			continue;

		if (min == -1 || line_compare(&chunks[i]->current_line,
					      &chunks[min]->current_line) < 0)
			min = i;
	}

	return min;
}

int chunk_write(struct chunk_t *chunk)
{
	size_t i;

	if (chunk) {
		for (i = 0; i < chunk->nb_lines; i++) {
			if (!fputs(chunk->lines[i]->value, chunk->fp)) {
				perror("fputs");
				return -1;
			}
		}
	}

	return 0;
}

void chunk_clear(struct chunk_t *chunk)
{
	size_t i;

	if (chunk) {
		if (chunk->lines) {
			for (i = 0; i < chunk->nb_lines; i++)
				line_destroy(chunk->lines[i]);

			free(chunk->lines);
			chunk->lines = NULL;
		}

		if (chunk->current_line)
			line_destroy(chunk->current_line);

		chunk->nb_lines = 0;
	}
}

void chunk_destroy(struct chunk_t *chunk)
{
	if (chunk) {
		fclose(chunk->fp);
		chunk_clear(chunk);
		free(chunk);
	}
}
