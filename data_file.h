#ifndef _DATA_FILE_H_
#define _DATA_FILE_H_

#include "chunk.h"

struct data_file_t {
	char *input_path;
	char *output_path;
	struct chunk_t **chunks;
	size_t nb_chunks;
	ssize_t chunk_size;
	char field_delim;
	int key_field;
};

struct data_file_t *data_file_create(const char *input_path,
				     const char *output_path,
				     const ssize_t chunk_size,
				     const char field_delim,
				     const int key_field);
int data_file_sort(struct data_file_t *data_file);
void data_file_destroy(struct data_file_t *data_file);

#endif
