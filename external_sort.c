#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "data_file.h"
#include "mem.h"

#define INPUT_FILE		"/home/eric/dev/data/test.txt"
#define OUTPUT_FILE	 	"/home/eric/dev/data/test.txt.sorted"
#define FIELD_DELIM	 	';'
#define KEY_FIELD		1
#define HEADER			1
#define NR_THREADS		8

/* default chunk size */
static ssize_t chunk_size = (ssize_t) 100 * (ssize_t) 1024 * (ssize_t) 1024;

/**
 * @brief Add a chunk to a data file.
 * 
 * @param data_file 		data file
 *
 * @return new chunk
 */
static struct chunk *data_file_add_chunk(struct data_file *data_file)
{
	data_file->chunks = (struct chunk **) xrealloc(data_file->chunks, sizeof(struct chunk *) * (data_file->nr_chunks + 1));
	data_file->chunks[data_file->nr_chunks] = chunk_create(NULL, 1);
	if (!data_file->chunks[data_file->nr_chunks])
		return NULL;

	data_file->nr_chunks += 1;
	return data_file->chunks[data_file->nr_chunks - 1];
}

/**
 * @brief Divide a file and sort it.
 * 
 * @param data_file 		data file
 *
 * @return status
 */
static int data_file_divide_and_sort(struct data_file *data_file)
{
	struct chunk *current_chunk = NULL;
	char *line = NULL;
	size_t len, i;
	int ret = 0;

	/* get header lines */
	if (data_file->header > 0) {
		data_file->header_lines = (char **) xmalloc(sizeof(char *) * data_file->header);

		for (i = 0; i < data_file->header; i++) {
			if (getline(&line, &len, data_file->fp_in) == -1)
				goto out;

			data_file->header_lines[data_file->nr_header_lines++] = xstrdup(line);
		}
	}

	/* read input file line by line */
	while(getline(&line, &len, data_file->fp_in) != -1) {
		/* create new chunk if needed */
		if (!current_chunk) {
			current_chunk = data_file_add_chunk(data_file);
			if (!current_chunk) {
				ret = -1;
				goto out;
			}
		}

		/* add line to current chunk */
		chunk_add_line(current_chunk, line, data_file->field_delim, data_file->key_field);

		/* write chunk */
		if (current_chunk->size >= data_file->chunk_size) {
			ret = chunk_sort_write(current_chunk, data_file->nr_threads);
			current_chunk = NULL;
			if (ret)
				goto out;
		}
	}

	/* write last chunk */
	if (current_chunk)
		ret = chunk_sort_write(current_chunk, data_file->nr_threads);

out:
	if (line)
		free(line);

	return ret;
}

/**
 * @brief Merge and sort a data file.
 * 
 * @param data_file 		data file
 *
 * @return status
 */
static int data_file_merge_sort(struct data_file *data_file)
{
	struct chunk *global_chunk;
	int ret = 0;
	size_t i;

	/* write header lines */
	for (i = 0; i < data_file->nr_header_lines; i++)
		fputs(data_file->header_lines[i], data_file->fp_out);

	/* create a global chunk */
	global_chunk = chunk_create(data_file->fp_out, 0);

	/* rewind each chunk */
	for (i = 0; i < data_file->nr_chunks; i++) {
		if (fseek(data_file->chunks[i]->fp, 0, SEEK_SET) == -1) {
			perror("fseek");
			ret = -1;
			goto out;
		}
	}

	/* peek a line from each buffer */
	for (i = 0; i < data_file->nr_chunks; i++)
		chunk_peek_line(data_file->chunks[i], data_file->field_delim, data_file->key_field);

	while (1) {
		/* compute min line */
		i = chunk_min_line(data_file->chunks, data_file->nr_chunks);
		if (i == -1)
			break;

		/* write line to global chunk */
		chunk_add_line(global_chunk, data_file->chunks[i]->current_line.value, data_file->field_delim, data_file->key_field);

		/* peek a line from min chunk */
		chunk_peek_line(data_file->chunks[i], data_file->field_delim, data_file->key_field);

		/* write chunk */
		if (global_chunk->size >= data_file->chunk_size) {
			ret = chunk_write(global_chunk);
			if (ret)
				goto out;
		}
	}

	/* write last chunk */
	if (global_chunk->size > 0)
		ret = chunk_write(global_chunk);

out:
	chunk_free(global_chunk);
	return ret;
}

/**
 * @brief Sort a file.
 * 
 * @param input_file 		input file
 * @param output_file 		output file
 * @param chunk_size 		chunk size
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 * @param header 		number of header lines
 * @param nr_threads		number of threads to use
 *
 * @return status
 */
static int sort(const char *input_file, const char *output_file, ssize_t chunk_size, char field_delim, int key_field, int header, size_t nr_threads)
{
	struct data_file *data_file;
	int ret;

	/* remove output file */
	remove(output_file);

	/* create data file */
	data_file = data_file_create(input_file, output_file, field_delim, key_field, header, nr_threads, chunk_size);

	/* divide and sort */
	ret = data_file_divide_and_sort(data_file);
	if (ret)
		goto out;

	/* only one chunk : no need to merge */
	if (data_file->nr_chunks <= 1)
		goto out;

	/* merge sort */
	ret = data_file_merge_sort(data_file);

out:
	/* free data file */
	data_file_free(data_file);

	return ret;
}

/**
 * @brief Main.
 * 
 * @return status
 */
int main(int argc, char **argv)
{
	return sort(INPUT_FILE, OUTPUT_FILE, chunk_size, FIELD_DELIM, KEY_FIELD, HEADER, NR_THREADS);
}
