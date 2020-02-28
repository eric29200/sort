#ifndef _LINE_H_
#define _LINE_H_

struct line_t {
	char *value;
	char *key;
	size_t key_len;
};

struct line_t *line_create(const char *value, char field_delim, int field_key);
int line_compare(const void *l1, const void *l2);
void line_destroy(struct line_t *line);

#endif
