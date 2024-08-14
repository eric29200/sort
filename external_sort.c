#include "chunk.h"
#include "buffered_reader.h"
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
 * @brief Divide and sort a file.
 * 
 * @param input_file		input file
 * @param fp_out		output file
 * @param chunk_size 		chunk size
 * @param field_delim		field delimiter
 * @param key_field		key field
 * @param header		number of header lines
 * @param nr_threads		number of threads to use
 *
 * @return chunks
 */
static struct chunk *__divide_and_sort(const char *input_file, FILE *fp_out, ssize_t chunk_size, char field_delim, int key_field, size_t header, size_t nr_threads)
{
	struct chunk *head = NULL, *chunk;
	struct buffered_reader *br = NULL;
	FILE *fp_in = NULL;
	size_t i;
	int ret;

	/* open input file */
	fp_in = fopen(input_file, "r");
	if (!fp_in) {
		fprintf(stderr, "Can't open input file \"%s\"\n", input_file);
		goto err;
	}

	/* create buffered reader */
	br = buffered_reader_create(fp_in, field_delim, key_field, header, chunk_size);
	if (!br)
		goto err;

	/* write header */
	for (i = 0; i < br->nr_header_lines; i++)
		fputs(br->header_lines[i], fp_out);

	/* divide and sort */
	for (;;) {
		/* create a new chunk */
		chunk = chunk_create();

		/* read chunk */
		buffered_reader_read_lines(br, chunk->larr);
		if (chunk->larr->size == 0) {
			chunk_free(chunk);
			break;
		}

		/* add chunk to list */
		chunk->next = head;
		head = chunk;

		/* sort and write chunk */
		ret = chunk_sort_write(chunk, nr_threads);
		if (ret)
			goto err;

		/* clear chunk */
		chunk_clear(chunk);
	}

	return head;
err:
	/* free chunks */
	for (chunk = head; chunk != NULL; chunk = chunk->next)
		chunk_free(chunk);

	/* close input file */
	if (fp_in)
		fclose(fp_in);

	return NULL;
}

/**
 * @brief Merge and sort a list of chunks.
 * 
 * @param fp			output file
 * @param field_delim		field delimiter
 * @param key_field		key field
 * @param chunk_size		chunk size
 * 
 * @return status
 */
static int __merge_sort(FILE *fp, struct chunk *chunks, char field_delim, int key_field, ssize_t chunk_size)
{
	size_t nr_chunks = 0;
	struct chunk *chunk;
	int len;

	/* get number of chunks */
	for (chunk = chunks; chunk != NULL; chunk = chunk->next)
		nr_chunks++;

	/* prepare read */
	for (chunk = chunks; chunk != NULL; chunk = chunk->next)
		chunk_prepare_read(chunk, field_delim, key_field, chunk_size / nr_chunks);

	/* merge chunks */
	for (;;) {
		/* compute min line */
		chunk = chunk_min_line(chunks);
		if (!chunk)
			break;

		/* write line to output file */
		len = (int) fwrite(chunk->current_line.value, 1, chunk->current_line.value_len, fp);
		if (len != chunk->current_line.value_len)
			return -1;

		/* peek a line from min chunk */
		chunk_peek_line(chunk);
	}

	return 0;
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
static int sort(const char *input_file, const char *output_file, ssize_t chunk_size, char field_delim, int key_field, size_t header, size_t nr_threads)
{
	struct chunk *chunks = NULL, *chunk;
	FILE *fp_out = NULL;
	int ret = -1;

	/* remove output file */
	remove(output_file);
	
	/* open input file */
	fp_out = fopen(output_file, "w");
	if (!fp_out) {
		fprintf(stderr, "Can't open output file \"%s\"\n", output_file);
		goto out;
	}
	
	/* divide and sort */
	chunks = __divide_and_sort(input_file, fp_out, chunk_size, field_delim, key_field, header, nr_threads);
	if (!chunks)
		goto out;

	/* merge sort */
	ret = __merge_sort(fp_out, chunks, field_delim, key_field, chunk_size);
out:
	/* free chunks */
	for (chunk = chunks; chunk != NULL; chunk = chunk->next)
		chunk_free(chunk);

	/* close output file */
	if (fp_out)
		fclose(fp_out);

	return ret;
}

int main()
{
	return sort(INPUT_FILE, OUTPUT_FILE, chunk_size, FIELD_DELIM, KEY_FIELD, HEADER, NR_THREADS);
}