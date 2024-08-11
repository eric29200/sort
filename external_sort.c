#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "chunk.h"
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
 * @brief Copy header.
 * 
 * @param fp_in 		input file
 * @param fp_out 		output file
 * @param header 		number of header lines
 *
 * @return status
 */
static int __copy_header(FILE *fp_in, FILE *fp_out, int header)
{
	char *line = NULL;
	size_t len;
	int i;

	for (i = 0; i < header; i++) {
		if (getline(&line, &len, fp_in) == -1)
			break;

		if (fputs(line, fp_out) == -1)
			return -1;
	}

	return 0;
}

/**
 * @brief Divide and sort a file.
 * 
 * @param fp			input file
 * @param chunk_size 		chunk size
 * @param field_delim		field delimiter
 * @param key_field		key field
 * @param nr_threads		number of threads to use
 *
 * @return chunks
 */
static struct chunk *__divide_and_sort(FILE *fp, ssize_t chunk_size, char field_delim, int key_field, size_t nr_threads)
{
	struct chunk *head = NULL, *chunk;
	char *buf, *ptr, *s;
	size_t len, off = 0;
	int ret;

	/* allocate buffer */
	buf = xmalloc(chunk_size + 1);

	for (;;) {
		/* read next chunk */
		len = fread(buf + off, 1, chunk_size - off, fp);
		if (len <= 0 && off <= 0)
			break;

		/* end chunk */
		buf[len + off] = 0;
		off = 0;
	
		/* create chunk */
		chunk = chunk_create(NULL, 1);
		chunk->next = head;
		head = chunk;

		/* parse content */
		for (s = buf; *s != 0;) {
			/* find end of line */
			ptr = strchrnul(s, '\n');

			/* add line */
			if (*ptr == '\n') {
				chunk_add_line(chunk, s, ptr - s + 1, field_delim, key_field);
				s = ptr + 1;
			} else {
				off = ptr - s;
				break;
			}
		}

		/* sort and write chunk */
		ret = chunk_sort_write(chunk, nr_threads);
		if (ret)
			goto err;

		/* remember last line */
		if (off > 0)
			memcpy(buf, s, off);
	}

	/* free buffer */
	xfree(buf);

	return head;
err:
	/* free buffer */
	xfree(buf);

	/* free chunks */
	for (chunk = head; chunk != NULL; chunk = chunk->next)
		chunk_free(chunk);

	return NULL;
}

/**
 * @brief Merge and sort a list of chunks.
 * 
 * @param fp			output file
 * @param field_delim		field delimiter
 * @param key_field		key field
 *
 * @return status
 */
static int __merge_sort(FILE *fp, struct chunk *chunks, char field_delim, int key_field)
{
	struct chunk *chunk;
	char *line = NULL;
	size_t len;
	int ret = -1;

	/* rewind each chunk */
	for (chunk = chunks; chunk != NULL; chunk = chunk->next) {
		if (fseek(chunk->fp, 0, SEEK_SET) == -1) {
			fprintf(stderr, "Can't rewind chunk\n");
			goto out;
		}
	}

	/* peek a line from each buffer */
	for (chunk = chunks; chunk != NULL; chunk = chunk->next)
		chunk_peek_line(chunk, &line, &len, field_delim, key_field);

	/* merge chunks */
	while (1) {
		/* compute min line */
		chunk = chunk_min_line(chunks);
		if (!chunk)
			break;

		/* write line to global chunk */
		fputs(chunk->current_line.value, fp);

		/* peek a line from min chunk */
		chunk_peek_line(chunk, &line, &len, field_delim, key_field);
	}

	ret = 0;
out:
	xfree(line);
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
	struct chunk *chunks = NULL, *chunk;
	FILE *fp_in = NULL, *fp_out = NULL;
	int ret = -1;

	/* remove output file */
	remove(output_file);
	
	/* open input file */
	fp_in = fopen(input_file, "r");
	if (!fp_in) {
		fprintf(stderr, "Can't open input file \"%s\"\n", input_file);
		goto out;
	}

	/* open input file */
	fp_out = fopen(output_file, "w");
	if (!fp_out) {
		fprintf(stderr, "Can't open output file \"%s\"\n", output_file);
		goto out;
	}

	/* copy header */
	if (__copy_header(fp_in, fp_out, header))
		goto out;

	/* divide and sort */
	chunks = __divide_and_sort(fp_in, chunk_size, field_delim, key_field, nr_threads);
	if (!chunks)
		goto out;

	/* merge sort */
	ret = __merge_sort(fp_out, chunks, field_delim, key_field);

out:
	/* free chunks */
	for (chunk = chunks; chunk != NULL; chunk = chunk->next)
		chunk_free(chunk);

	/* close input file */
	if (fp_in)
		fclose(fp_in);

	/* close output file */
	if (fp_out)
		fclose(fp_out);

	return ret;
}

/**
 * @brief Main.
 * 
 * @return status
 */
int main()
{
	return sort(INPUT_FILE, OUTPUT_FILE, chunk_size, FIELD_DELIM, KEY_FIELD, HEADER, NR_THREADS);
}
