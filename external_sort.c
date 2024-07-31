#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "mem.h"
#include "line.h"

#define INPUT_FILE		"/home/eric/dev/data/test.txt"
#define OUTPUT_FILE	 	"/home/eric/dev/data/test.txt.sorted"
#define FIELD_DELIM	 	';'
#define KEY_FIELD		1
#define HEADER			1

/* default chunk size */
static ssize_t chunk_size = (ssize_t) 512 * (ssize_t) 1024 * (ssize_t) 1024;

/**
 * @brief Chunk structure.
 */
struct chunk {
	FILE *			fp;
	struct line_array *	line_array;	
	ssize_t 		size;
	struct line 		current_line;
};

/**
 * @brief Data file.
 */
struct data_file {
	char *			input_file;
	char *			output_file;
	struct chunk **		chunks;
	size_t 			nr_chunks;
	ssize_t 		chunk_size;
	char 			field_delim;
	int 			key_field;
	int 			header;
	char **			header_lines;
	size_t			nr_header_lines;
};

/**
 * @brief Create a chunk.
 * 
 * @param fp 		chunk file
 *
 * @return chunk
 */
static struct chunk *chunk_create(FILE *fp)
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
 * @brief Add a line to a chunk.
 * 
 * @param chunk 		chunk
 * @param value 		line value
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 */
static void chunk_add_line(struct chunk *chunk, const char *value, char field_delim, int key_field)
{
	if (!chunk || !value)
		return;

	line_array_add(chunk->line_array, value, field_delim, key_field);
	chunk->size += strlen(value);
}

/**
 * @brief Peek a line from a chunk.
 * 
 * @param chunk 		chunk
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 */
static void chunk_peek_line(struct chunk *chunk, char field_delim, int key_field)
{
	char *line = NULL;
	size_t len;

	/* free previous line */
	line_free(&chunk->current_line);

	/* get next line */
	if (getline(&line, &len, chunk->fp) != -1) {
		line_init(&chunk->current_line, line, field_delim, key_field);
		free(line);
	}
}

/**
 * @brief Sort a chunk.
 * 
 * @param chunk 		chunk
 */
static void chunk_sort(struct chunk *chunk)
{
	line_array_sort(chunk->line_array);
}

/**
 * @brief Get minimum line from a list of chunks.
 * 
 * @param chunks 		chunks
 * @param nr_chunks 		number of chunks
 *
 * @return chunk index containing minimum line
 */
static int chunk_min_line(struct chunk **chunks, size_t nr_chunks)
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
static int chunk_write(struct chunk *chunk)
{
	size_t i;

	if (chunk) {
		for (i = 0; i < chunk->line_array->size; i++) {
			if (!fputs(chunk->line_array->lines[i].value, chunk->fp)) {
				perror("fputs");
				return -1;
			}
		}
	}

	return 0;
}

/**
 * @brief Clear a chunk.
 * 
 * @param chunk 		chunk
 */
static void chunk_clear(struct chunk *chunk)
{
	if (!chunk)
		return;

	/* clear lines */
	line_array_clear(chunk->line_array);
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
static void chunk_free(struct chunk *chunk)
{
	if (chunk) {
		fclose(chunk->fp);
		chunk_clear(chunk);
		free(chunk);
	}
}

/**
 * @brief Create a data file.
 * 
 * @param input_file 		input file
 * @param output_file 		output file
 * @param chunk_size 		chunk size
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 * @param header 		number of header lines
 *
 * @return data file
 */
static struct data_file *data_file_create(const char *input_file, const char *output_file, ssize_t chunk_size,
					  char field_delim, int key_field, int header)
{
	struct data_file *data_file;

	data_file = (struct data_file *) xmalloc(sizeof(struct data_file));
	data_file->input_file = xstrdup(input_file);
	data_file->output_file = xstrdup(output_file);
	data_file->chunks = NULL;
	data_file->nr_chunks = 0;
	data_file->chunk_size = chunk_size;
	data_file->field_delim = field_delim;
	data_file->key_field = key_field;
	data_file->header = header;
	data_file->header_lines = NULL;
	data_file->nr_header_lines = 0;

	return data_file;
}

/**
 * @brief Free a data file.
 * 
 * @param data_file 		data file
 */
static void data_file_free(struct data_file *data_file)
{
	size_t i;

	if (data_file) {
		xfree(data_file->input_file);
		xfree(data_file->output_file);

		/* free header lines */
		if (data_file->header_lines) {
			for (i = 0; i < data_file->nr_header_lines; i++)
				free(data_file->header_lines[i]);

			free(data_file->header_lines);
		}
		
		/* free chunks */
		if (data_file->chunks) {
			for (i = 0; i < data_file->nr_chunks; i++)
				chunk_free(data_file->chunks[i]);

			xfree(data_file->chunks);
		}
	}
}

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
	data_file->chunks[data_file->nr_chunks] = chunk_create(NULL);
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
	FILE *fp;

	/* open input file */
	fp = fopen(data_file->input_file, "r");
	if (!fp) {
		perror("fopen");
		return -1;
	}

	/* get header lines */
	if (data_file->header > 0) {
		data_file->header_lines = (char **) xmalloc(sizeof(char *) * data_file->header);

		for (i = 0; i < data_file->header; i++) {
			if (getline(&line, &len, fp) == -1)
				goto out;

			data_file->header_lines[data_file->nr_header_lines++] = xstrdup(line);
		}
	}

	/* read input file line by line */
	while(getline(&line, &len, fp) != -1) {
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
		if (data_file->nr_chunks == 1) {
			fclose(current_chunk->fp);
			current_chunk->fp = fopen(data_file->output_file, "w");
			if (!current_chunk->fp) {
				perror("fopen");
				ret = -1;
				goto out;
			}

			/* write header lines */
			for (i = 0; i < data_file->nr_header_lines; i++)
				fputs(data_file->header_lines[i], fp);
		}

		ret = chunk_write(current_chunk);
		chunk_clear(current_chunk);
	}
out:
	if (line)
		free(line);
	fclose(fp);
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
	FILE *fp;
	size_t i;

	/* open output file */
	fp = fopen(data_file->output_file, "w");
	if (!fp) {
		perror("fopen");
		return -1;
	}

	/* write header lines */
	for (i = 0; i < data_file->nr_header_lines; i++)
		fputs(data_file->header_lines[i], fp);

	/* create a global chunk */
	global_chunk = chunk_create(fp);

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
 *
 * @return status
 */
static int sort(const char *input_file, const char *output_file, ssize_t chunk_size, char field_delim, int key_field, int header)
{
	struct data_file *data_file;
	int ret;

	/* remove output file */
	remove(output_file);

	/* create data file */
	data_file = data_file_create(input_file, output_file, chunk_size, field_delim, key_field, header);

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
	return sort(INPUT_FILE, OUTPUT_FILE, chunk_size, FIELD_DELIM, KEY_FIELD, HEADER);
}
