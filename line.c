#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "line.h"

struct line_t *line_create(const char *value, char field_delim, int key_field)
{
	struct line_t *line;
	char *kend;

	line = (struct line_t *) sort_malloc(sizeof(struct line_t));
	line->value = sort_strdup(value);

	/* find key start */
	line->key = line->value;
	while (key_field-- && (line->key = strchr(line->key, field_delim)))
	       line->key += 1;

	/* find key end */
	kend = strchr(line->key, field_delim);

	/* compute key length */
	if (line->key)
		line->key_len = kend ? kend - line->key : strlen(line->key);
	else
		line->key_len = 0;

	return line;
}

int line_compare(const void *l1, const void *l2)
{
	struct line_t *line1;
	struct line_t *line2;
	size_t len;
	int ret;

	line1 = *((struct line_t **) l1);
	line2 = *((struct line_t **) l2);

	len = line1->key_len < line2->key_len ? line1->key_len : line2->key_len;

	ret = strncmp(line1->key, line2->key, len);
	if (ret)
		return ret;

	if (line1->key_len < line2->key_len)
		return -1;
	else if (line1->key_len > line2->key_len)
		return 1;

	return 0;
}

void line_destroy(struct line_t *line)
{
	if (line) {
		sort_free(line->value);
		sort_free(line);
	}
}
