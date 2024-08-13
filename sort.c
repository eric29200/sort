#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

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
	FILE *fp_in = NULL, *fp_out = NULL;
	struct buffered_reader *br = NULL;
	struct line_array *larr = NULL;
	int ret = -1;
	size_t i;

	/* remove output file */
	remove(output_file);

	/* open input file */
	fp_in = fopen(input_file, "r");
	if (!fp_in) {
		fprintf(stderr, "Can't open input file \"%s\"\n", output_file);
		goto out;
	}

	/* create buffered reader */
	br = buffered_reader_create(fp_in, field_delim, key_field, 0);
	if (!br)
		goto out;

	/* read header */
	buffered_reader_read_header(br, header);

	/* read lines */
	larr = line_array_create();
	buffered_reader_read_lines(br, larr);

	/* open output file */
	fp_out = fopen(output_file, "w");
	if (!fp_out) {
		fprintf(stderr, "Can't open output file \"%s\"\n", output_file);
		goto out;
	}

	/* write header */
	for (i = 0; i < br->nr_header_lines; i++)
		fputs(br->header_lines[i], fp_out);

	/* sort lines */
	line_array_sort(larr, nr_threads);

	/* write lines */
	ret = line_array_write(larr, fp_out);
out:
	/* free lines */
	if (larr)
		line_array_free(larr);

	/* free buffered reader */
	if (br)
		buffered_reader_free(br);

	/* close input file */
	if (fp_in)
		fclose(fp_in);
	
	/* close output file */
	if (fp_out)
		fclose(fp_out);

	return ret;
}

int main()
{
	return sort(INPUT_FILE, OUTPUT_FILE, FIELD_DELIM, KEY_FIELD, HEADER, NR_THREADS);
}