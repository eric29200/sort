#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "data_file.h"

struct data_file_t *data_file_create(const char *path, const ssize_t chunk_size,
				     const char field_delim, const int key_field)
{
	struct data_file_t *data_file;

	data_file = (struct data_file_t *) malloc(sizeof(struct data_file_t));
	if (!data_file) {
		perror("malloc");
		return NULL;
	}

	data_file->path = strdup(path);
	if (!data_file->path) {
		perror("strdup");
		free(data_file);
		return NULL;
	}

	data_file->nb_chunks = 0;
	data_file->chunk_size = chunk_size;
	data_file->field_delim = field_delim;
	data_file->key_field = key_field;

	return data_file;
}

static struct chunk_t *data_file_add_chunk(struct data_file_t *data_file)
{
	data_file->chunks = (struct chunk_t **) realloc(data_file->chunks,
							sizeof(struct chunk_t *)
							* (data_file->nb_chunks
							   + 1));
	if (!data_file->chunks) {
		perror("realloc");
		return NULL;
	}

	data_file->chunks[data_file->nb_chunks] = chunk_create();
	if (!data_file->chunks[data_file->nb_chunks])
		return NULL;

	data_file->nb_chunks += 1;
	return data_file->chunks[data_file->nb_chunks - 1];
}

static int data_file_divide_and_sort(struct data_file_t *data_file)
{
	FILE *fp;
	char line[LINE_SIZE];
	struct chunk_t *current_chunk = NULL;
	int ret = 0;

	fp = fopen(data_file->path, "r");
	if (!fp) {
		perror("fopen");
		return -1;
	}

	while(fgets(line, LINE_SIZE, fp)) {
		/* create new chunk if needed */
		if (!current_chunk) {
			current_chunk = data_file_add_chunk(data_file);
			if (!current_chunk) {
				ret = -ENOMEM;
				goto out;
			}
		}

		/* add line to current chunk */
		chunk_add_line(current_chunk, line, data_file->field_delim,
			       data_file->key_field);

		/* write chunk */
		if (current_chunk->size >= data_file->chunk_size) {
			chunk_sort(current_chunk, line_compar);
			ret = chunk_write(current_chunk);
			chunk_clear(current_chunk);
			current_chunk = NULL;
			if (ret)
				goto out;
		}
	}

	/* write last chunk */
	if (current_chunk) {
		chunk_sort(current_chunk, line_compar);
		ret = chunk_write(current_chunk);
		chunk_clear(current_chunk);
	}
out:
	fclose(fp);
	return ret;
}

static int data_file_merge(struct data_file_t *data_file)
{
	size_t i;
	char line[LINE_SIZE];
	int ret = 0;

	printf("%ld\n", data_file->nb_chunks);

	/* rewind each chunk */
	for (i = 0; i < data_file->nb_chunks; i++) {
		if (fseek(data_file->chunks[i]->fp, 0, SEEK_SET) == -1) {
			perror("fseek");
			return -1;
		}
	}

	/* TODO */

	/* peek a line from each chunk */
	for (i = 0; i < data_file->nb_chunks; i++) {
		fgets(line, LINE_SIZE, data_file->chunks[i]->fp);
	}

	/* TODO */

	return ret;
}

int data_file_sort(struct data_file_t *data_file)
{
	int ret;

	ret = data_file_divide_and_sort(data_file);
	if (ret)
		return ret;

	ret = data_file_merge(data_file);

	return ret;
}

void data_file_destroy(struct data_file_t *data_file)
{
	size_t i;

	if (data_file) {
		if (data_file->path)
			free(data_file->path);

		if (data_file->chunks) {
			for (i = 0; i < data_file->nb_chunks; i++)
				free(data_file->chunks[i]);

			free(data_file->chunks);
		}
	}
}
