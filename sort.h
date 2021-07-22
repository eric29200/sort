#ifndef _SORT_H_
#define _SORT_H_

#include "mem.h"

int sort(const char *input_path, const char *output_path, ssize_t chunk_size,
         char field_delim, int key_field, int header);

#endif
