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
	char 			field_delim;
	int 			key_field;
	int 			header;
	char *			content;
	struct line_array *	line_array;	
	char **			header_lines;
	size_t			nr_header_lines;
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
	data_file->field_delim = field_delim;
	data_file->key_field = key_field;
	data_file->header = header;
	data_file->content = NULL;
	data_file->line_array = line_array_create();
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
		xfree(data_file->content);

		/* free header lines */
		if (data_file->header_lines) {
			for (i = 0; i < data_file->nr_header_lines; i++)
				free(data_file->header_lines[i]);

			free(data_file->header_lines);
		}

		/* free lines */
		line_array_free(data_file->line_array);
	}
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
	char *ptr, *s;
	long fp_len;
	int ret = 0;
	FILE *fp;

	/* open input file */
	fp = fopen(data_file->input_file, "r");
	if (!fp) {
		perror("fopen");
		return -1;
	}

	/* get file length */
	fseek(fp, 0, SEEK_END);
	fp_len = ftell(fp);
	rewind(fp);

	/* read file */
	data_file->content = xmalloc(fp_len);
	if (fread(data_file->content, fp_len, 1, fp) != 1) {
		perror("fread");
		ret = -1;
		goto out;
	}

	/* parse content */
	for (ptr = data_file->content, s = data_file->content; *ptr != 0; ptr++) {
		/* new line */
		if (*ptr == '\n') {
			/* end of line */
			*ptr = 0;

			/* add line */
			line_array_add(data_file->line_array, s, data_file->field_delim, data_file->key_field);

			/* go to next line */
			s = ptr + 1;
		}
	}

out:
	/* close input file */
	fclose(fp);

	return ret;
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
	for (i = 0; i < data_file->line_array->size; i++)
		fputs(data_file->line_array->lines[i].value, fp);

	/* close output file */
	fclose(fp);

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
	line_array_sort(data_file->line_array);

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
