#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#define INPUT_FILE		"/home/eric/dev/data/test.txt"
#define OUTPUT_FILE	 	"/home/eric/dev/data/test.txt.sorted"
#define FIELD_DELIM	 	'\t'
#define KEY_FIELD		3
#define HEADER			1

/*
 * Line structure.
 */
struct line_t {
	char *			value;
	char *			key;
	size_t 			key_len;
};

/*
 * Data file.
 */
struct data_file_t {
	char *			input_path;
	struct line_t **	lines;
	size_t 			nb_lines;
	char 			field_delim;
	int 			key_field;
	int 			header;
};

/*
 * Malloc or exit.
 */
void *xmalloc(size_t size)
{
	void *ptr;

	ptr = malloc(size);
	if (!ptr)
		err(2, NULL);

	return ptr;
}

/*
 * Calloc or exit.
 */
void *xcalloc(size_t nmemb, size_t size)
{
	void *ptr;

	ptr = calloc(nmemb, size);
	if (!ptr)
		err(2, NULL);

	return ptr;
}

/*
 * Realloc or exit.
 */
void *xrealloc(void *ptr, size_t size)
{
	ptr = realloc(ptr, size);
	if (!ptr)
		err(2, NULL);

	return ptr;
}

/*
 * Free memory.
 */
void xfree(void *ptr)
{
	if (ptr)
		free(ptr);
}

/*
 * Strdup or exit.
 */
void *xstrdup(const char *s)
{
	char *dup;

	dup = strdup(s);
	if (!dup)
		err(2, NULL);

	return dup;
}

/*
 * Create a line.
 */
static struct line_t *line_create(const char *value, char field_delim, int key_field)
{
	struct line_t *line;
	char *kend;

	line = (struct line_t *) xmalloc(sizeof(struct line_t));
	line->value = xstrdup(value);

	/* find key start */
	line->key = line->value;
	while (key_field-- && (line->key = strchr(line->key, field_delim)))
		line->key += 1;

	/* compute key end and length */
	if (line->key) {
		kend = strchr(line->key, field_delim);
		line->key_len = kend ? kend - line->key : strlen(line->key);
	} else {
		line->key_len = 0;
	}

	return line;
}

/*
 * Compare 2 lines.
 */
static inline int line_compare(const void *l1, const void *l2)
{
	struct line_t *line1 = *((struct line_t **) l1);
	struct line_t *line2 = *((struct line_t **) l2);
	size_t len;
	int i;

	len = line1->key_len < line2->key_len ? line1->key_len : line2->key_len;

	for (i = 0; i < len; i++)
		if (line1->key[i] != line2->key[i])
			return line1->key[i] - line2->key[i];

	return line1->key_len - line2->key_len;
}

/*
 * Destroy a line.
 */
static void line_destroy(struct line_t *line)
{
	if (line) {
		xfree(line->value);
		free(line);
	}
}

/*
 * Create a data file.
 */
static struct data_file_t *data_file_create(const char *input_path, char field_delim, int key_field, int header)
{
	struct data_file_t *data_file;

	data_file = (struct data_file_t *) xmalloc(sizeof(struct data_file_t));
	data_file->input_path = xstrdup(input_path);
	data_file->lines = NULL;
	data_file->nb_lines = 0;
	data_file->field_delim = field_delim;
	data_file->key_field = key_field;
	data_file->header = header;

	return data_file;
}

/*
 * Destroy a data file.
 */
static void data_file_destroy(struct data_file_t *data_file)
{
	size_t i;

	if (data_file) {
		xfree(data_file->input_path);

		if (data_file->lines) {
			for (i = 0; i < data_file->nb_lines; i++)
				line_destroy(data_file->lines[i]);

			xfree(data_file->lines);
		}
	}
}

/*
 * Add a line to a data file.
 */
static void data_file_add_line(struct data_file_t *data_file, const char *value, char field_delim, int key_field)
{
	struct line_t *new_line;

	if (!data_file || !value)
		return;

	new_line = line_create(value, field_delim, key_field);
	data_file->lines = (struct line_t **) xrealloc(data_file->lines, sizeof(struct line_t *) * (data_file->nb_lines + 1));
	data_file->lines[data_file->nb_lines] = new_line;
	data_file->nb_lines += 1;
}

/*
 * Read a data file.
 */
static int data_file_read(struct data_file_t *data_file)
{
	char *line = NULL;
	size_t len;
	FILE *fp;

	/* open input file */
	fp = fopen(data_file->input_path, "r");
	if (!fp) {
		perror("fopen");
		return -1;
	}

	/* skip header */
	if (data_file->header > 0)
		getline(&line, &len, fp);

	/* read input file line by line */
	while(getline(&line, &len, fp) != -1)
		data_file_add_line(data_file, line, data_file->field_delim, data_file->key_field);

	/* close input file */
	fclose(fp);
	return 0;
}

/*
 * Sort a data file.
 */
static int data_file_sort(struct data_file_t *data_file)
{
	int ret;

	/* read data file */
	ret = data_file_read(data_file);
	if (ret)
		return ret;

	/* sort data file */
	if (data_file->nb_lines > 0)
		qsort(data_file->lines, data_file->nb_lines, sizeof(struct line_t *), line_compare);

	return ret;
}

/*
 * Sort a file.
 */
static int sort(const char *input_path, const char *output_path, char field_delim, int key_field, int header)
{
	struct data_file_t *data_file;
	int ret;

	/* create data file */
	data_file = data_file_create(input_path, field_delim, key_field, header);

	/* sort */
	ret = data_file_sort(data_file);

	/* destroy data file */
	data_file_destroy(data_file);
	return ret;
}

/*
 * Main.
 */
int main(int argc, char **argv)
{
	return sort(INPUT_FILE, OUTPUT_FILE, FIELD_DELIM, KEY_FIELD, HEADER);
}
