#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "buffered_reader.h"
#include "mem.h"

/**
 * @brief Read header.
 * 
 * @param br 			buffered reader
 * @param header 		number of header lines
 */
static void __read_header(struct buffered_reader *br, size_t header)
{
	char *line = NULL;
	size_t len = 0, i;

	/* allocate header lines */
	br->header_lines = (char **) xmalloc(header * sizeof(char *));

	/* read header lines */
	for (i = 0; i < header; i++) {
		if (getline(&line, &len, br->fp) == -1)
			break;

		br->header_lines[br->nr_header_lines++] = xstrdup(line);
	}
}

/**
 * @brief Estimate line length.
 * 
 * @param br 			buffered reader
 *
 * @return line length
 */
static size_t __estimate_line_length(struct buffered_reader *br)
{
	char *line = NULL;
	ssize_t line_len;
	size_t len;

	/* read first line */
	line_len = getline(&line, &len, br->fp);
	if (line_len <= 0) {
		line_len = 0;
		goto out;
	}

	/* rewind */
	fseek(br->fp, -line_len, SEEK_CUR);

out:
	/* free memory */
	xfree(line);

	return line_len;
}

/**
 * @brief Create a buffered reader.
 * 
 * @param fp			input file
 * @param field_delim		field delimiter
 * @param key_field		key field
 * @param header		number of header lines
 * @param memory_size		memory size
 * 
 * @return buffered reader
 */
struct buffered_reader *buffered_reader_create(FILE *fp, char field_delim, int key_field, size_t header, ssize_t memory_size)
{
	struct buffered_reader *br;
	struct stat st;

	/* allocate reader */
	br = (struct buffered_reader *) xmalloc(sizeof(struct buffered_reader));
	br->field_delim = field_delim;
	br->key_field = key_field;
	br->fp = fp;
	br->buf = NULL;
	br->buf_len = 0;
	br->off = 0;
	br->header_lines = NULL;
	br->nr_header_lines = 0;
	
	/* read header */
	if (header > 0)
		__read_header(br, header);

	/* estimate line length */
	br->line_len = __estimate_line_length(br);
	if (br->line_len <= 0) {
		fprintf(stderr, "Can't estimate line length\n");
		goto err;
	}

	/* set buffer capacity */
	if (memory_size <= 0) {
		if (fstat(fileno(br->fp), &st)) {
			fprintf(stderr, "Can't stat input file\n");
			goto err;
		}

		br->buf_capacity = st.st_size;
	} else {
		br->buf_capacity = memory_size - (memory_size / br->line_len) * sizeof(struct line);
	}

	/* allocate buffer */
	br->buf = (char *) xmalloc(br->buf_capacity + 1);

	return br;
err:
	buffered_reader_free(br);
	return NULL;
}

/**
 * @brief Free a buffered reader.
 * 
 * @param br 			buffered reader
 */
void buffered_reader_free(struct buffered_reader *br)
{
	size_t i;

	if (!br)
		return;

	/* free header lines */
	if (br->header_lines) {
		for (i = 0; i < br->nr_header_lines; i++)
			free(br->header_lines[i]);

		free(br->header_lines);
	}

	/* free memory */
	xfree(br->buf);
	free(br);
}

/**
 * @brief Read next lines.
 * 
 * @param br 			buffered reader
 * @param larr			lines array
 */
void buffered_reader_read_lines(struct buffered_reader *br, struct line_array *larr)
{
	char *ptr = NULL, *s;
	size_t len;

	/* copy last line */
	memcpy(br->buf, br->buf + br->buf_len - br->off, br->off);

	/* read next chunk */
	len = fread(br->buf + br->off, 1, br->buf_capacity - br->off, br->fp);
	if (len <= 0)
		return;

	/* end buffer */
	br->buf_len = br->off + len;
	br->buf[br->buf_len] = 0;
	br->off = 0;

	/* parse content */
	for (s = br->buf; *s != 0;) {
		/* find end of line */
		ptr = strchrnul(s, '\n');

		/* end of buf */
		if (!*ptr)
			break;

		/* add line */
		line_array_add(larr, s, ptr - s + 1, br->field_delim, br->key_field);

		/* go to next line */
		s = ptr + 1;
	}

	/* save last line */
	if (ptr && ptr > s)
		br->off = ptr - s;
}