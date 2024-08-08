#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "data_file.h"

#define INPUT_FILE		"/home/eric/dev/data/test.txt"
#define OUTPUT_FILE	 	"/home/eric/dev/data/test.txt.sorted"
#define FIELD_DELIM	 	';'
#define KEY_FIELD		1
#define HEADER			1
#define NR_THREADS		8

/**
 * @brief Read a data file.
 * 
 * @param data_file 		data file
 *
 * @return status
 */
static int __data_file_read(struct data_file *data_file)
{
	char *ptr, *s;
	long fp_len;
	size_t i;

	/* get file length */
	fseek(data_file->fp_in, 0, SEEK_END);
	fp_len = ftell(data_file->fp_in);
	rewind(data_file->fp_in);

	/* read file */
	data_file->content = ptr = xmalloc(fp_len);
	if (fread(data_file->content, fp_len, 1, data_file->fp_in) != 1) {
		perror("fread");
		return -1;
	}

	/* parse header */
	for (i = 0; i < data_file->header; i++) {
		/* find end of line */
		for (s = ptr; *ptr != '\n' && *ptr != 0; ptr++);
		if (*ptr == '\n')
			ptr++;
	}

	/* set header length */
	data_file->header_len = ptr - data_file->content;

	/* parse content */
	for (s = ptr; *ptr != 0; ptr++) {
		/* new line */
		if (*ptr == '\n') {
			/* add line */
			line_array_add(data_file->line_array, s, ptr - s, data_file->field_delim, data_file->key_field);

			/* go to next line */
			s = ptr + 1;
		}
	}

	return 0;
}

/**
 * @brief Write a data file.
 * 
 * @param data_file 		data file
 *
 * @return status
 */
static int __data_file_write(struct data_file *data_file)
{
	size_t i;

	/* write header */
	fwrite(data_file->content, data_file->header_len, 1, data_file->fp_out);

	/* write lines */
	for (i = 0; i < data_file->line_array->size; i++) {
		fwrite(data_file->line_array->lines[i].value, data_file->line_array->lines[i].value_len, 1, data_file->fp_out);
		fputc('\n', data_file->fp_out);
	}

	return 0;
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
	struct data_file *data_file;
	int ret;

	/* remove output file */
	remove(output_file);

	/* create data file */
	data_file = data_file_create(input_file, output_file, field_delim, key_field, header, nr_threads, 0);
	
	/* read data file */
	ret = __data_file_read(data_file);
	if (ret)
		goto out;

	/* sort data file */
	line_array_sort(data_file->line_array, data_file->nr_threads);

	/* write data file */
	ret = __data_file_write(data_file);

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
int main()
{
	return sort(INPUT_FILE, OUTPUT_FILE, FIELD_DELIM, KEY_FIELD, HEADER, NR_THREADS);
}
