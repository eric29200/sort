#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "line.h"
#include "mem.h"

#define INPUT_FILE		"/home/eric/dev/data/test.txt"
#define OUTPUT_FILE	 	"/home/eric/dev/data/test.txt.sorted"
#define FIELD_DELIM	 	';'
#define KEY_FIELD		1
#define HEADER			1
#define NR_THREADS		8

/**
 * @brief Read input file.
 * 
 * @param input_file 		input file
 * @param len			input file length
 *
 * @return file content
 */
static char *__read_input_file(const char *input_file, size_t *len)
{
	char *buf = NULL;
	struct stat st;
	FILE *fp;

	/* open input file */
	fp = fopen(input_file, "r");
	if (!fp) {
		fprintf(stderr, "Can't open input file \"%s\"\n", input_file);
		goto out;
	}

	/* get file size */
	if (fstat(fileno(fp), &st)) {
		fprintf(stderr, "Can't stat input file \"%s\"\n", input_file);
		goto out;
	}

	/* allocate buffer */
	*len = st.st_size;
	buf = xmalloc(st.st_size + 1);

	/* read file */
	if (fread(buf, st.st_size, 1, fp) != 1) {
		fprintf(stderr, "Can't read input file \"%s\"\n", input_file);
		xfree(buf);
		goto out;
	}

	/* end buffer */
	buf[st.st_size] = 0;

out:
	/* close input file */
	if (fp)
		fclose(fp);

	return buf;
}

/**
 * @brief Write output file.
 * 
 * @param output_file 		output file
 * @param output_file_len	output file length
 * @param header 		header
 * @param header_len 		header length
 * @param larr 			lines array
 *
 * @return status
 */
static int __write_output_file(const char *output_file, size_t output_file_len, const char *header, size_t header_len, struct line_array *larr)
{
	int ret = -1;
	FILE *fp;
	size_t i;

	/* open output file */
	fp = fopen(output_file, "w");
	if (!fp) {
		fprintf(stderr, "Can't open output file \"%s\"\n", output_file);
		goto out;
	}

	/* allocate output file */
	if (fallocate(fileno(fp), 0, 0, output_file_len)) {
		fprintf(stderr, "Can't allocate output file \"%s\"\n", output_file);
		goto out;
	}

	/* write header */
	if (fwrite(header, header_len, 1, fp) != 1) {
		fprintf(stderr, "Can't write output file \"%s\"\n", output_file);
		goto out;
	}

	/* write content */
	for (i = 0; i < larr->size; i++)
		if (fwrite(larr->lines[i].value, larr->lines[i].value_len, 1, fp) != 1)
			goto out;

	ret = 0;
out:
	/* close output file */
	fclose(fp);

	return ret;
}

/**
 * @brief Parse header.
 * 
 * @param buf 			input file buffer
 * @param header 		number of header lines
 *
 * @return header length
 */
static size_t __parse_header(const char *buf, int header)
{
	const char *ptr;
	int i;
	
	for (i = 0, ptr = buf; i < header; i++) {
		ptr = strchrnul(ptr, '\n');
		if (*ptr == '\n')
			ptr++;
		else
			break;
	}

	return ptr - buf;
}

/**
 * @brief Parse content.
 * 
 * @param buf 			input file buffer
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 *
 * @return content
 */
static struct line_array *__parse_content(char *buf, char field_delim, int key_field)
{
	struct line_array *larr;
	char *ptr, *s;

	/* create line array */
	larr = line_array_create();

	/* parse content */
	for (s = buf; *s != 0;) {
		/* find end of line */
		ptr = strchrnul(s, '\n');

		/* add line */
		if (*ptr == '\n') {
			line_array_add(larr, s, ptr - s + 1, field_delim, key_field);
			s = ptr + 1;
		} else {
			line_array_add(larr, s, ptr - s, field_delim, key_field);
			break;
		}
	}

	return larr;
}

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
static int sort(const char *input_file, const char *output_file, char field_delim, int key_field, int header, size_t nr_threads)
{
	size_t header_len, buf_len;
	struct line_array *larr;
	char *buf;
	int ret;

	/* remove output file */
	remove(output_file);

	/* load input file */
	buf = __read_input_file(input_file, &buf_len);
	if (!buf)
		return -1;

	/* parse header */
	header_len = __parse_header(buf, header);

	/* parse content */
	larr = __parse_content(buf + header_len, field_delim, key_field);

	/* sort content */
	line_array_sort(larr, nr_threads);

	/* write output file */
	ret = __write_output_file(output_file, buf_len, buf, header_len, larr);

	/* free memory */
	line_array_free(larr);
	free(buf);

	return ret;
}

/**
 * @brief Main.
 * 
 * @return status
 */
int main()
{
	return sort(INPUT_FILE, OUTPUT_FILE, FIELD_DELIM, KEY_FIELD, HEADER, NR_THREADS);
}
