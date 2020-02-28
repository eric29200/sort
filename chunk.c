#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "chunk.h"

static int chunk_id = 1;

struct chunk_t *chunk_create()
{
	struct chunk_t *chunk;

	chunk = (struct chunk_t *) malloc(sizeof(struct chunk_t));
	if (!chunk) {
		perror("malloc");
		return NULL;
	}

	chunk->id = chunk_id++;
	chunk->fp = tmpfile();
	if (!chunk->fp) {
		perror("tmpfile");
		free(chunk);
		return NULL;
	}
	chunk->lines = NULL;
	chunk->nb_lines = 0;
	chunk->size = 0;

	return chunk;
}

int chunk_add_line(struct chunk_t *chunk, const char *line,
		   const char field_delim, const int key_field)
{
	struct line_t *new_line;

	if (!chunk || !line)
		return -1;

	chunk->lines = (struct line_t **) realloc(chunk->lines,
						  sizeof(struct line_t *)
						  * (chunk->nb_lines + 1));
	if (!chunk->lines) {
		perror("malloc");
		return -ENOMEM;
	}

	new_line = line_create(line, field_delim, key_field);
	if (!new_line) {
		perror("strdup");
		return -ENOMEM;
	}


	chunk->lines[chunk->nb_lines] = new_line;
	chunk->nb_lines += 1;
	chunk->size += sizeof(new_line) + strlen(new_line->value)
		+ strlen(new_line->key);
	return 0;
}

void chunk_sort(struct chunk_t *chunk, int (*compar)(const void *, const void *))
{
	if (chunk && chunk->nb_lines > 0)
		qsort(chunk->lines, chunk->nb_lines, sizeof(struct line_t *),
		      compar);
}

int chunk_write(struct chunk_t *chunk)
{
	size_t i;

	if (!chunk)
		return -1;

	for (i = 0; i < chunk->nb_lines; i++) {
		if (!fputs(chunk->lines[i]->value, chunk->fp)) {
			perror("fputs");
			return -1;
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
