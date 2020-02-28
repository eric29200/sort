#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "line.h"

static char *line_extract_key(const char *value, const char field_delim,
			      const int key_field)
{
	size_t i;
	size_t start = -1;
	size_t end = -1;
	int j = key_field;
	char *key;

	for (i = 0; value[i] != 0 && j >= 0; i++) {
		if (value[i] == field_delim) {
			if (--j == 0)
				start = i + 1;
			else if (j == -1)
				end = i;
		}
	}

	if (start == -1 || end == -1)
		return NULL;

	key = (char *) calloc(end - start + 1, sizeof(char));
	if (!key)
		return NULL;

	strncpy(key, value + start, end - start);

	return key;
}

struct line_t *line_create(const char *value, const char field_delim,
			   const int key_field)
{
	struct line_t *line;

	line = (struct line_t *) malloc(sizeof(struct line_t));
	if (!line) {
		perror("malloc");
		return NULL;
	}

	line->value = strdup(value);
	if (!line->value) {
		perror("strdup");
		free(line);
		return NULL;
	}

	line->key = line_extract_key(value, field_delim, key_field);

	return line;
}

int line_compar(const void *line1, const void *line2)
{
	return strcmp(((struct line_t *) line1)->key,
		      ((struct line_t *) line2)->key);
}

void line_destroy(struct line_t *line)
{
	if (line) {
		if (line->key)
			free(line->key);

		if (line->value)
			free(line->value);

		free(line);
	}
}
