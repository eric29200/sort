#ifndef _SORT_H_
#define _SORT_H_

#include "mem.h"

#define LINE_SIZE	4096

struct line_t {
	char *value;
	char *key;
	size_t key_len;
};

struct chunk_t {
	FILE *fp;
	struct line_t **lines;
	size_t nb_lines;
	ssize_t size;
	struct line_t *current_line;
};

struct data_file_t {
	char *input_path;
	char *output_path;
	struct chunk_t **chunks;
	size_t nb_chunks;
	ssize_t chunk_size;
	char field_delim;
	int key_field;
	int header;
	char header_line[LINE_SIZE];
};

int sort(const char *input_path, const char *output_path, ssize_t chunk_size,
	 char field_delim, int key_field, int header);

#endif
