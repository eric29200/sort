#ifndef _CHUNK_H_
#define _CHUNK_H_

#include "line.h"

struct chunk_t {
	FILE *fp;
	struct line_t **lines;
	size_t nb_lines;
	ssize_t size;
};

struct chunk_t *chunk_create(FILE *fp_output);
void chunk_add_line(struct chunk_t *chunk, const char *line,
		   char field_delim, int key_field);
void chunk_sort(struct chunk_t *chunk);
int chunk_write(struct chunk_t *chunk);
void chunk_clear(struct chunk_t *chunk);
void chunk_destroy(struct chunk_t *chunk);


#endif
