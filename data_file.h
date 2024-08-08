#ifndef _DATA_FILE_H_
#define _DATA_FILE_H_

#include <sys/types.h>

#include "chunk.h"

/**
 * @brief Data file.
 */
struct data_file {
	FILE *			fp_in;
	FILE *			fp_out;
	char 			field_delim;
	int 			key_field;
	int 			header;
	size_t			nr_threads;
	ssize_t			chunk_size;
	char *			content;
	struct line_array *	line_array;	
	struct chunk **		chunks;
	size_t			nr_chunks;
	char **			header_lines;
	size_t			nr_header_lines;
	size_t			header_len;
};

/**
 * @brief Create a data file.
 * 
 * @param input_file		input file
 * @param output_file		output file
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 * @param header 		number of header lines
 * @param nr_threads		number of threads to use
 * @param chunk_size		chunk size
 *
 * @return data file
 */
struct data_file *data_file_create(const char *input_file, const char *output_file, char field_delim,
				   int key_field, int header, size_t nr_threads, ssize_t chunk_size);

/**
 * @brief Free a data file.
 * 
 * @param data_file 		data file
 */
void data_file_free(struct data_file *data_file);

#endif