#ifndef MIMEDOWN_WRAP_H
#define MIMEDOWN_WRAP_H

#include <stddef.h>
#include "vendor/queue.h"

struct wordlist_entry
{
	const char *start;
	ptrdiff_t len;

	TAILQ_ENTRY(wordlist_entry) entries;
};
TAILQ_HEAD(wordlist, wordlist_entry);

struct wordlist *build_wordlist(const char *text);
void free_wordlist(struct wordlist *ws);
void print_wrapped(const struct wordlist *ws, const int width);

#endif
