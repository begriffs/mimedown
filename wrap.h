#ifndef MIMEDOWN_WRAP_H
#define MIMEDOWN_WRAP_H

#include <stdbool.h>
#include <stddef.h>

struct wordlist_entry
{
	const char *start;
	ptrdiff_t len;
};

struct wordlist
{
	struct wordlist_entry *words;
	size_t len, _alloced;
};

struct wordlist *wordlist_create(void);

void wordlist_append_copy(struct wordlist *ws, const struct wordlist_entry *w);
struct wordlist *wordlist_segment(struct wordlist *ws, const char *text);
void wordlist_concat(struct wordlist *dst, const struct wordlist *src);
void wordlist_free(struct wordlist *ws);
size_t print_wrapped(const struct wordlist *ws, const char *overhang,
		size_t width, bool flowed);

#endif
