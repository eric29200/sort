#include "data_file.h"
#include "mem.h"

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
				   int key_field, int header, size_t nr_threads, ssize_t chunk_size)
{
	struct data_file *data_file;

	data_file = (struct data_file *) xmalloc(sizeof(struct data_file));
	data_file->fp_in = NULL;
	data_file->fp_out = NULL;
	data_file->field_delim = field_delim;
	data_file->key_field = key_field;
	data_file->header = header;
	data_file->nr_threads = nr_threads;
	data_file->chunk_size = chunk_size;
	data_file->content = NULL;
	data_file->line_array = line_array_create();
	data_file->chunks = NULL;
	data_file->nr_chunks = 0;
	data_file->header_lines = NULL;
	data_file->nr_header_lines = 0;
	data_file->header_len = 0;

	/* open input file */
	data_file->fp_in = fopen(input_file, "r");
	if (!data_file->fp_in) {
		fprintf(stderr, "Can't open input file \"%s\"\n", input_file);
		goto err;
	}

	/* open output file */
	data_file->fp_out = fopen(output_file, "w");
	if (!data_file->fp_out) {
		fprintf(stderr, "Can't open output file \"%s\"\n", output_file);
		goto err;
	}

	return data_file;
err:
	data_file_free(data_file);
	return NULL;
}

/**
 * @brief Free a data file.
 * 
 * @param data_file 		data file
 */
void data_file_free(struct data_file *data_file)
{
	size_t i;

	if (!data_file)
		return;

	/* close input file */
	if (data_file->fp_in)
		fclose(data_file->fp_in);

	/* close output file */
	if (data_file->fp_out)
		fclose(data_file->fp_out);

	/* free memory */
	xfree(data_file->content);
	line_array_free(data_file->line_array);

	/* free header lines */
	if (data_file->header_lines) {
		for (i = 0; i < data_file->nr_header_lines; i++)
			free(data_file->header_lines[i]);

		free(data_file->header_lines);
	}
	
	/* free chunks */
	if (data_file->chunks) {
		for (i = 0; i < data_file->nr_chunks; i++)
			chunk_free(data_file->chunks[i]);

		free(data_file->chunks);
	}
}