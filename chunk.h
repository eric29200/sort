#ifndef _CHUNK_H_
#define _CHUNK_H_

#include "line.h"

struct chunk_t {
	int id;
	FILE *fp;
	struct line_t **lines;
	size_t nb_lines;
	ssize_t size;
};

struct chunk_t *chunk_create();
int chunk_add_line(struct chunk_t *chunk, const char *line,
		   const char field_delim, const int key_field);
void chunk_sort(struct chunk_t *chunk,
		int (*compar)(const void *, const void *));
int chunk_write(struct chunk_t *chunk);
void chunk_clear(struct chunk_t *chunk);
void chunk_destroy(struct chunk_t *chunk);


#endif
