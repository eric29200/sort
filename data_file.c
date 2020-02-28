#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "data_file.h"

struct data_file_t *data_file_create(const char *input_path,
				     const char *output_path,
				     const ssize_t chunk_size,
				     const char field_delim, const int key_field)
{
	struct data_file_t *data_file;

	data_file = (struct data_file_t *) malloc(sizeof(struct data_file_t));
	if (!data_file) {
		perror("malloc");
		return NULL;
	}

	data_file->input_path = strdup(input_path);
	if (!data_file->input_path) {
		perror("strdup");
		free(data_file);
		return NULL;
	}

	data_file->output_path = strdup(output_path);
	if (!data_file->output_path) {
		perror("strdup");
		free(data_file->input_path);
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

	data_file->chunks[data_file->nb_chunks] = chunk_create(NULL);
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

	/* open input file */
	fp = fopen(data_file->input_path, "r");
	if (!fp) {
		perror("fopen");
		return -1;
	}

	/* read input file line by line */
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
		ret = chunk_add_line(current_chunk, line,
				     data_file->field_delim,
				     data_file->key_field);
		if (ret)
			goto out;

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

int data_file_merge_sort(struct data_file_t *data_file)
{
	FILE *fp_output;
	struct chunk_t *chunk;
	char *chunks_status;
	size_t nb_remaining_chunks;
	char line[LINE_SIZE];
	size_t i;
	int ret = 0;

	/* open output file */
	fp_output = fopen(data_file->output_path, "w");
	if (!fp_output) {
		perror("fopen");
		return -1;
	}

	/* create output chunk */
	chunk = chunk_create(fp_output);
	if (!chunk) {
		fclose(fp_output);
		return -1;
	}

	/* create an array with chunks status */
	nb_remaining_chunks = data_file->nb_chunks;
	chunks_status = (char *) malloc(sizeof(char) * nb_remaining_chunks);
	if (!chunks_status) {
		perror("malloc");
		chunk_destroy(chunk);
		return -1;
	}

	/* init chunks list */
	for (i = 0; i < data_file->nb_chunks; i++)
		chunks_status[i] = 1;

	/* rewind each chunk */
	for (i = 0; i < data_file->nb_chunks; i++) {
		if (fseek(data_file->chunks[i]->fp, 0, SEEK_SET) == -1) {
			perror("fseek");
			ret = -1;
			goto out;
		}
	}

	while (nb_remaining_chunks > 0) {
		/* peek a line of each chunk */
		for (i = 0; i < data_file->nb_chunks; i++) {
			if (chunks_status[i] == 0)
				continue;

			if (fgets(line, LINE_SIZE, data_file->chunks[i]->fp)) {
				/* add line to chunk */
				ret = chunk_add_line(chunk, line,
						     data_file->field_delim,
						     data_file->key_field);
				if (ret)
					goto out;
			} else {
				nb_remaining_chunks--;
				chunks_status[i] = 0;
			}
		}

		/* if chunk is full, compute it */
		if (chunk->size >= data_file->chunk_size) {
			chunk_sort(chunk, line_compar);
			ret = chunk_write(chunk);
			if (ret)
				goto out;
			chunk_clear(chunk);
		}
	}

	/* compute last chunk */
	if (chunk->size > 0) {
		chunk_sort(chunk, line_compar);
		ret = chunk_write(chunk);
		if (ret)
			goto out;
		chunk_clear(chunk);
	}
out:
	free(chunks_status);
	chunk_destroy(chunk);
	return ret;
}

int data_file_sort(struct data_file_t *data_file)
{
	int ret;

	ret = data_file_divide_and_sort(data_file);
	if (ret)
		return ret;

	return data_file_merge_sort(data_file);
}

void data_file_destroy(struct data_file_t *data_file)
{
	size_t i;

	if (data_file) {
		if (data_file->input_path)
			free(data_file->input_path);

		if (data_file->output_path)
			free(data_file->output_path);

		if (data_file->chunks) {
			for (i = 0; i < data_file->nb_chunks; i++)
				free(data_file->chunks[i]);

			free(data_file->chunks);
		}
	}
}
