#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "chunk.h"
#include "buffered_reader.h"
#include "mem.h"

#define INPUT_FILE		"/home/eric/dev/data/test.txt"
#define OUTPUT_FILE	 	"/home/eric/dev/data/test.txt.sorted"
#define FIELD_DELIM	 	';'
#define KEY_FIELD		1
#define HEADER			1
#define NR_THREADS		8

/**
 * @brief Sort a file.
 * 
 * @param input_file 		input file
 * @param output_file 		output file
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 * @param header 		number of header lines
 * @param nr_threads		number of threads to use
 *
 * @return status
 */
static int sort(const char *input_file, const char *output_file, char field_delim, int key_field, size_t header, size_t nr_threads)
{
	struct buffered_reader *br = NULL;
	struct chunk *chunk = NULL;
	int ret = -1;
	size_t i;

	/* remove output file */
	remove(output_file);

	/* create buffered reader */
	br = buffered_reader_create(input_file, field_delim, key_field, 0);
	if (!br)
		goto out;

	/* read header */
	buffered_reader_read_header(br, header);

	/* read chunk */
	chunk = buffered_reader_read_chunk(br);
	if(!chunk)
		goto out;

	/* open output file */
	chunk->fp = fopen(output_file, "w");
	if (!chunk->fp) {
		fprintf(stderr, "Can't open output file \"%s\"\n", output_file);
		goto out;
	}

	/* write header */
	for (i = 0; i < br->nr_header_lines; i++)
		fputs(br->header_lines[i], chunk->fp);

	/* sort and write chunk */
	ret = chunk_sort_write(chunk, nr_threads);
out:
	/* free chunk */
	if (chunk)
		chunk_free(chunk);

	/* free buffered reader */
	if (br)
		buffered_reader_free(br);

	return ret;
}

int main()
{
	return sort(INPUT_FILE, OUTPUT_FILE, FIELD_DELIM, KEY_FIELD, HEADER, NR_THREADS);
}