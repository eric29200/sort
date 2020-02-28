#ifndef _LINE_H_
#define _LINE_H_

struct line_t {
	char *key;
	char *value;
};

struct line_t *line_create(const char *value, const char field_delim,
			   const int field_key);
int line_compar(const void *l1, const void *l2);
void line_destroy(struct line_t *line);

#endif
