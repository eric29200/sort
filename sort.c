#include <stdio.h>
#include <errno.h>

#include "data_file.h"

#define INPUT_FILE	"/home/eric/dev/data/test.txt"
#define OUTPUT_FILE	"/home/eric/dev/data/test.txt.sorted"
#define FIELD_DELIM	'\t'
#define KEY_FIELD	6

static ssize_t chunk_size = (ssize_t) 1000 * (ssize_t) 1024 * (ssize_t) 1000;

int main(int argc, char **argv)
{
	struct data_file_t *data_file;
	int ret = 0;

	/* create data file */
	data_file = data_file_create(INPUT_FILE, OUTPUT_FILE, chunk_size,
				     FIELD_DELIM, KEY_FIELD);
	if (!data_file) {
		ret = -ENOMEM;
		goto out;
	}

	/* sort data file */
	ret = data_file_sort(data_file);
	if (ret)
		goto release;
release:
	data_file_destroy(data_file);
out:
	return ret;
}
