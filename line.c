#include <string.h>
#include <stdlib.h>

#include "line.h"
#include "mem.h"

/**
 * @brief Init a line.
 * 
 * @param line			line
 * @param value 		line value
 * @param field_delim 		field delimiter
 * @param key_field 		key field
 */
void line_init(struct line *line, const char *value, char field_delim, int key_field)
{
	char *kend;

	/* set value */
	line->value = xstrdup(value);

	/* find key start */
	line->key = line->value;
	while (key_field-- && (line->key = strchr(line->key, field_delim)))
		line->key++;

	/* compute key end and length */
	if (line->key) {
		kend = strchr(line->key, field_delim);
		line->key_len = kend ? kend - line->key : strlen(line->key);
	} else {
		line->key_len = 0;
	}
}

/**
 * @brief Free a line.
 * 
 * @param line 		line
 */
void line_free(struct line *line)
{
	free(line->value);
	line->value = NULL;
}

/**
 * @brief Compare 2 lines.
 * 
 * @param l1 		first line
 * @param l2 		second line
 *
 * @return comparison result
 */
int line_compare(const void *l1, const void *l2)
{
	struct line *line1 = (struct line *) l1;
	struct line *line2 = (struct line *) l2;
	size_t len;
	int ret;

	/* find maximum length */
	len = line1->key_len < line2->key_len ? line1->key_len : line2->key_len;

	/* compare keys */
	ret = strncmp(line1->key, line2->key, len);
	if (ret)
		return ret;

	return line1->key_len - line2->key_len;
}