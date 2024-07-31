#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "line.h"

#define INPUT_FILE		"/home/eric/dev/data/test.txt"
#define OUTPUT_FILE	 	"/home/eric/dev/data/test.txt.sorted"
#define FIELD_DELIM	 	';'
#define KEY_FIELD		1
#define HEADER			1

/**
 * @brief Data file.
 */
struct data_file {
	char *			input_file;
	char *			output_file;
	struct line *		lines;
	size_t 			nr_lines;
	size_t			lines_capacity;
	char **			header_lines;
	size_t			nr_header_lines;
	char 			field_delim;
	int 			key_field;
	int 			header;
};

/**
 * @brief Create a data file.
 * 
 * @param input_file		input file
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 * @param header 		number of header lines
 *
 * @return data file
 */
static struct data_file *data_file_create(const char *input_file, const char *output_file, char field_delim, int key_field, int header)
{
	struct data_file *data_file;

	data_file = (struct data_file *) xmalloc(sizeof(struct data_file));
	data_file->input_file = xstrdup(input_file);
	data_file->output_file = xstrdup(output_file);
	data_file->lines = NULL;
	data_file->nr_lines = 0;
	data_file->lines_capacity = 0;
	data_file->header_lines = NULL;
	data_file->nr_header_lines = 0;
	data_file->field_delim = field_delim;
	data_file->key_field = key_field;
	data_file->header = header;

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

		/* free lines */
		if (data_file->lines) {
			for (i = 0; i < data_file->nr_lines; i++)
				line_free(&data_file->lines[i]);

			free(data_file->lines);
		}
	}
}

/**
 * @brief Add a line to a data file.
 * 
 * @param data_file 		data file
 * @param value 		line value
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 */
static void data_file_add_line(struct data_file *data_file, const char *value, char field_delim, int key_field)
{
	if (!data_file || !value)
		return;

	/* grow lines array if needed */
	if (data_file->nr_lines == data_file->lines_capacity) {
		data_file->lines_capacity = data_file->lines_capacity + (data_file->lines_capacity >> 1);
		if (data_file->lines_capacity < 10)
			data_file->lines_capacity = 10;
		data_file->lines = (struct line *) xrealloc(data_file->lines, sizeof(struct line) * data_file->lines_capacity);
	}

	/* add line */
	line_init(&data_file->lines[data_file->nr_lines], value, field_delim, key_field);
	data_file->nr_lines += 1;
}

/**
 * @brief Read a data file.
 * 
 * @param data_file 		data file
 *
 * @return status
 */
static int data_file_read(struct data_file *data_file)
{
	char *line = NULL;
	size_t len;
	FILE *fp;
	int i;

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
	while (getline(&line, &len, fp) != -1)
		data_file_add_line(data_file, line, data_file->field_delim, data_file->key_field);

out:
	/* close input file */
	fclose(fp);

	return 0;
}

/**
 * @brief Write a data file.
 * 
 * @param data_file 		data file
 *
 * @return status
 */
static int data_file_write(struct data_file *data_file)
{
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

	/* write lines */
	for (i = 0; i < data_file->nr_lines; i++)
		fputs(data_file->lines[i].value, fp);

	/* close output file */
	fclose(fp);

	return 0;
}

/**
 * @brief Sort a data file.
 * 
 * @param data_file 		data file
 */
static void data_file_sort(struct data_file *data_file)
{
	if (data_file->nr_lines > 0)
		qsort(data_file->lines, data_file->nr_lines, sizeof(struct line), line_compare);
}

/**
 * @brief Sort a file.
 * 
 * @param input_file 		input file
 * @param output_file 		output file
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 * @param header 		number of header lines
 *
 * @return status
 */
static int sort(const char *input_file, const char *output_file, char field_delim, int key_field, int header)
{
	struct data_file *data_file;
	int ret;

	/* remove output file */
	remove(output_file);

	/* create data file */
	data_file = data_file_create(input_file, output_file, field_delim, key_field, header);
	
	/* read data file */
	ret = data_file_read(data_file);
	if (ret)
		goto out;

	/* sort data file */
	data_file_sort(data_file);

	/* write data file */
	ret = data_file_write(data_file);

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
	return sort(INPUT_FILE, OUTPUT_FILE, FIELD_DELIM, KEY_FIELD, HEADER);
}
