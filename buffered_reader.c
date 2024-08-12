#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "buffered_reader.h"
#include "mem.h"

/**
 * @brief Create a buffered reader.
 * 
 * @param fp			input file
 * @param field_delim		field delimiter
 * @param key_field		key field
 * @param chunk_size		chunk size
 * 
 * @return buffered reader
 */
struct buffered_reader *buffered_reader_create(FILE *fp, char field_delim, int key_field, ssize_t chunk_size)
{
	struct buffered_reader *br;
	struct stat st;

	/* allocate reader */
	br = (struct buffered_reader *) xmalloc(sizeof(struct buffered_reader));
	br->field_delim = field_delim;
	br->key_field = key_field;
	br->chunk_size = chunk_size;
	br->fp = fp;
	br->buf = NULL;
	br->buf_len = 0;
	br->off = 0;
	br->header_lines = NULL;
	br->nr_header_lines = 0;

	/* fix chunk size */
	if (chunk_size <= 0) {
		if (fstat(fileno(br->fp), &st)) {
			fprintf(stderr, "Can't stat input file\n");
			goto err;
		}

		br->chunk_size = st.st_size;
	}

	/* allocate buffer */
	br->buf = (char *) xmalloc(br->chunk_size + 1);

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
 * @brief Read header.
 * 
 * @param br 			buffered reader
 * @param header 		number of header lines
 */
void buffered_reader_read_header(struct buffered_reader *br, size_t header)
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
 * @brief Read next chunk.
 * 
 * @param br 			buffered reader
 *
 * @return next chunk
 */
struct chunk *buffered_reader_read_chunk(struct buffered_reader *br)
{
	char *ptr = NULL, *s;
	struct chunk *chunk;
	size_t len;

	/* copy last line */
	memcpy(br->buf, br->buf + br->buf_len - br->off, br->off);

	/* read next chunk */
	len = fread(br->buf + br->off, 1, br->chunk_size - br->off, br->fp);
	if (len <= 0)
		return NULL;

	/* end buffer */
	br->buf_len = br->off + len;
	br->buf[br->buf_len] = 0;
	br->off = 0;

	/* create a new chunk */
	chunk = chunk_create();

	/* parse content */
	for (s = br->buf; *s != 0;) {
		/* find end of line */
		ptr = strchrnul(s, '\n');

		/* end of buf */
		if (!*ptr)
			break;

		/* add line */
		line_array_add(chunk->larr, s, ptr - s + 1, br->field_delim, br->key_field);
		s = ptr + 1;
	}

	/* save last line */
	if (ptr && ptr > s)
		br->off = ptr - s;

	return chunk;
}