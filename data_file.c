#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "mem.h"
#include "data_file.h"

struct data_file_t *data_file_create(const char *input_path,
				     const char *output_path,
				     ssize_t chunk_size,
				     char field_delim, int key_field, int header)
{
	struct data_file_t *data_file;

	data_file = (struct data_file_t *)
		sort_malloc(sizeof(struct data_file_t));
	data_file->input_path = sort_strdup(input_path);
	data_file->output_path = sort_strdup(output_path);
	data_file->nb_chunks = 0;
	data_file->chunk_size = chunk_size;
	data_file->field_delim = field_delim;
	data_file->key_field = key_field;
	data_file->header = header;

	return data_file;
}

static struct chunk_t *data_file_add_chunk(struct data_file_t *data_file)
{
	data_file->chunks = (struct chunk_t **) sort_realloc(data_file->chunks,
							sizeof(struct chunk_t *)
							* (data_file->nb_chunks
							   + 1));
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

	/* store header */
	if (data_file->header > 0)
		fgets(data_file->header_line, LINE_SIZE, fp);

	/* read input file line by line */
	while(fgets(line, LINE_SIZE, fp)) {
		/* create new chunk if needed */
		if (!current_chunk) {
			current_chunk = data_file_add_chunk(data_file);
			if (!current_chunk) {
				ret = -1;
				goto out;
			}
		}

		/* add line to current chunk */
		chunk_add_line(current_chunk, line, data_file->field_delim,
			       data_file->key_field);

		/* write chunk */
		if (current_chunk->size >= data_file->chunk_size) {
			chunk_sort(current_chunk);
			ret = chunk_write(current_chunk);
			chunk_clear(current_chunk);
			current_chunk = NULL;
			if (ret)
				goto out;
		}
	}

	/* write last chunk */
	if (current_chunk) {
		chunk_sort(current_chunk);

		/* if only one chunk : write directly to the output */
		if (data_file->nb_chunks == 1) {
			fclose(current_chunk->fp);
			current_chunk->fp = fopen(data_file->output_path, "w");
			if (!current_chunk->fp) {
				perror("fopen");
				ret = -1;
				goto out;
			}
		}

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
	struct chunk_t *global_chunk;
	size_t i;
	int ret = 0;

	/* open output file */
	fp_output = fopen(data_file->output_path, "w");
	if (!fp_output) {
		perror("fopen");
		return -1;
	}

	/* write header */
	if (data_file->header > 0)
		fputs(data_file->header_line, fp_output);

	/* create a global chunk */
	global_chunk = chunk_create(fp_output);

	/* rewind each chunk */
	for (i = 0; i < data_file->nb_chunks; i++) {
		if (fseek(data_file->chunks[i]->fp, 0, SEEK_SET) == -1) {
			perror("fseek");
			ret = -1;
			goto out;
		}
	}

	/* peek a line from each buffer */
	for (i = 0; i < data_file->nb_chunks; i++)
		chunk_peek_line(data_file->chunks[i], data_file->field_delim,
				data_file->key_field);

	while (1) {
		/* compute min line */
		i = chunk_min_line(data_file->chunks, data_file->nb_chunks);
		if (i == -1)
			break;

		/* write line to global chunk */
		chunk_add_line(global_chunk,
			       data_file->chunks[i]->current_line->value,
			       data_file->field_delim, data_file->key_field);

		/* peek a line from min chunk */
		chunk_peek_line(data_file->chunks[i], data_file->field_delim,
				data_file->key_field);

		/* write chunk */
		if (global_chunk->size >= data_file->chunk_size) {
			ret = chunk_write(global_chunk);
			chunk_clear(global_chunk);
			if (ret)
				goto out;
		}
	}

	/* write last chunk */
	if (global_chunk->size > 0) {
		ret = chunk_write(global_chunk);
		chunk_clear(global_chunk);
	}
out:
	chunk_destroy(global_chunk);
	return ret;
}

int data_file_sort(struct data_file_t *data_file)
{
	int ret;

	ret = data_file_divide_and_sort(data_file);
	if (ret)
		return ret;

	/* only one chunk : no need to merge */
	if (data_file->nb_chunks <= 1)
		return 0;

	return data_file_merge_sort(data_file);
}

void data_file_destroy(struct data_file_t *data_file)
{
	size_t i;

	if (data_file) {
		sort_free(data_file->input_path);
		sort_free(data_file->output_path);
		if (data_file->chunks) {
			for (i = 0; i < data_file->nb_chunks; i++)
				sort_free(data_file->chunks[i]);

			sort_free(data_file->chunks);
		}
	}
}
