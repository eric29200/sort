#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#define INPUT_FILE		"/home/eric/dev/data/test.txt"
#define OUTPUT_FILE	 	"/home/eric/dev/data/test.txt.sorted"
#define FIELD_DELIM	 	'\t'
#define KEY_FIELD		3
#define HEADER			1

/* default chunk size */
static ssize_t chunk_size = (ssize_t) 512 * (ssize_t) 1024 * (ssize_t) 1024;

/*
 * Line structure.
 */
struct line_t {
	char *			value;
	char *			key;
	size_t 			key_len;
};

/*
 * Chunk structure.
 */
struct chunk_t {
	FILE *			fp;
	struct line_t **	lines;
	size_t 			nb_lines;
	ssize_t 		size;
	struct line_t *		current_line;
};

/*
 * Data file.
 */
struct data_file_t {
	char *			input_path;
	char *			output_path;
	struct chunk_t **	chunks;
	size_t 			nb_chunks;
	ssize_t 		chunk_size;
	char 			field_delim;
	int 			key_field;
	int 			header;
	char *			header_line;
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
 * Create a chunk.
 */
static struct chunk_t *chunk_create(FILE *fp)
{
	struct chunk_t *chunk;

	chunk = xmalloc(sizeof(struct chunk_t));
	chunk->lines = NULL;
	chunk->nb_lines = 0;
	chunk->size = 0;
	chunk->current_line = NULL;

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

/*
 * Add a line to a chunk.
 */
static void chunk_add_line(struct chunk_t *chunk, const char *value, char field_delim, int key_field)
{
	struct line_t *new_line;

	if (!chunk || !value)
		return;

	new_line = line_create(value, field_delim, key_field);
	chunk->lines = (struct line_t **) xrealloc(chunk->lines, sizeof(struct line_t *) * (chunk->nb_lines + 1));
	chunk->lines[chunk->nb_lines] = new_line;
	chunk->nb_lines += 1;
	chunk->size += sizeof(new_line) + strlen(new_line->value);
}

/*
 * Peek a line from a chunk.
 */
static void chunk_peek_line(struct chunk_t *chunk, char field_delim, int key_field)
{
	char *line = NULL;
	size_t len;

	/* destroy previous line */
	line_destroy(chunk->current_line);

	if (getline(&line, &len, chunk->fp) != -1) {
		chunk->current_line = line_create(line, field_delim, key_field);
		free(line);
	} else {
		chunk->current_line = NULL;
	}
}

/*
 * Sort a chunk.
 */
static void chunk_sort(struct chunk_t *chunk)
{
	if (chunk && chunk->nb_lines > 0)
		qsort(chunk->lines, chunk->nb_lines, sizeof(struct line_t *), line_compare);
}

/*
 * Get minimum link from a list of chunks.
 */
static int chunk_min_line(struct chunk_t **chunks, size_t nb_chunks)
{
	size_t i;
	int min = -1;

	for (i = 0; i < nb_chunks; i++) {
		if (!chunks[i]->current_line)
			continue;

		if (min == -1 || line_compare(&chunks[i]->current_line, &chunks[min]->current_line) < 0)
			min = i;
	}

	return min;
}

/*
 * Write a chunk.
 */
static int chunk_write(struct chunk_t *chunk)
{
	size_t i;

	if (chunk) {
		for (i = 0; i < chunk->nb_lines; i++) {
			if (!fputs(chunk->lines[i]->value, chunk->fp)) {
				perror("fputs");
				return -1;
			}
		}
	}

	return 0;
}

/*
 * Clear a chunk.
 */
static void chunk_clear(struct chunk_t *chunk)
{
	size_t i;

	if (chunk) {
		if (chunk->lines) {
			for (i = 0; i < chunk->nb_lines; i++)
				line_destroy(chunk->lines[i]);

			free(chunk->lines);
			chunk->lines = NULL;
		}

		if (chunk->current_line)
			line_destroy(chunk->current_line);

		chunk->nb_lines = 0;
	}
}

/*
 * Destroy a chunk.
 */
static void chunk_destroy(struct chunk_t *chunk)
{
	if (chunk) {
		fclose(chunk->fp);
		chunk_clear(chunk);
		free(chunk);
	}
}

/*
 * Create a data file.
 */
static struct data_file_t *data_file_create(const char *input_path, const char *output_path, ssize_t chunk_size,
					    char field_delim, int key_field, int header)
{
	struct data_file_t *data_file;

	data_file = (struct data_file_t *) xmalloc(sizeof(struct data_file_t));
	data_file->input_path = xstrdup(input_path);
	data_file->output_path = xstrdup(output_path);
	data_file->nb_chunks = 0;
	data_file->chunk_size = chunk_size;
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
		xfree(data_file->output_path);
		xfree(data_file->header_line);
		if (data_file->chunks) {
			for (i = 0; i < data_file->nb_chunks; i++)
				chunk_destroy(data_file->chunks[i]);

			xfree(data_file->chunks);
		}
	}
}

/*
 * Add a chunk to a data file.
 */
static struct chunk_t *data_file_add_chunk(struct data_file_t *data_file)
{
	data_file->chunks = (struct chunk_t **) xrealloc(data_file->chunks, sizeof(struct chunk_t *) * (data_file->nb_chunks + 1));
	data_file->chunks[data_file->nb_chunks] = chunk_create(NULL);
	if (!data_file->chunks[data_file->nb_chunks])
		return NULL;

	data_file->nb_chunks += 1;
	return data_file->chunks[data_file->nb_chunks - 1];
}

/*
 * Divide a data file and sort it.
 */
static int data_file_divide_and_sort(struct data_file_t *data_file)
{
	struct chunk_t *current_chunk = NULL;
	FILE *fp;
	char *line = NULL;
	size_t len;
	int ret = 0;

	/* open input file */
	fp = fopen(data_file->input_path, "r");
	if (!fp) {
		perror("fopen");
		return -1;
	}

	/* store header */
	if (data_file->header > 0)
		getline(&data_file->header_line, &len, fp);

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
		if (data_file->nb_chunks == 1) {
			fclose(current_chunk->fp);
			current_chunk->fp = fopen(data_file->output_path, "w");
			if (!current_chunk->fp) {
				perror("fopen");
				ret = -1;
				goto out;
			}

			/* write header */
			if (data_file->header > 0)
				fputs(data_file->header_line, current_chunk->fp);
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

/*
 * Merge and sort a data file.
 */
static int data_file_merge_sort(struct data_file_t *data_file)
{
	FILE *fp_output;
	struct chunk_t *global_chunk;
	size_t i;
	int ret = 0;

	/* open output file */
	fp_output = fopen(data_file->output_path, "w");
	if (!fp_output) {
		perror("fopen");
		return -1;
	}

	/* write header */
	if (data_file->header > 0)
		fputs(data_file->header_line, fp_output);

	/* create a global chunk */
	global_chunk = chunk_create(fp_output);

	/* rewind each chunk */
	for (i = 0; i < data_file->nb_chunks; i++) {
		if (fseek(data_file->chunks[i]->fp, 0, SEEK_SET) == -1) {
			perror("fseek");
			ret = -1;
			goto out;
		}
	}

	/* peek a line from each buffer */
	for (i = 0; i < data_file->nb_chunks; i++)
		chunk_peek_line(data_file->chunks[i], data_file->field_delim, data_file->key_field);

	while (1) {
		/* compute min line */
		i = chunk_min_line(data_file->chunks, data_file->nb_chunks);
		if (i == -1)
			break;

		/* write line to global chunk */
		chunk_add_line(global_chunk, data_file->chunks[i]->current_line->value, data_file->field_delim, data_file->key_field);

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
	chunk_destroy(global_chunk);
	return ret;
}

/*
 * Sort a file.
 */
static int sort(const char *input_path, const char *output_path, ssize_t chunk_size, char field_delim, int key_field, int header)
{
	struct data_file_t *data_file;
	int ret;

	/* create data file */
	data_file = data_file_create(input_path, output_path, chunk_size, field_delim, key_field, header);

	/* divide and sort */
	ret = data_file_divide_and_sort(data_file);
	if (ret)
		goto out;

	/* only one chunk : no need to merge */
	if (data_file->nb_chunks <= 1)
		goto out;

	/* merge sort */
	ret = data_file_merge_sort(data_file);
out:
	data_file_destroy(data_file);
	return ret;
}

/*
 * Main.
 */
int main(int argc, char **argv)
{
	return sort(INPUT_FILE, OUTPUT_FILE, chunk_size, FIELD_DELIM, KEY_FIELD, HEADER);
}
