#include "wrap.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(x,y) ((x<y) ? (y) : (x))

static void _wordlist_invariant(const struct wordlist *ws)
{
	assert(ws);
#ifndef NDEBUG
	struct wordlist_entry *w;
	TAILQ_FOREACH(w, ws, entries)
		assert(w->len >= 0);
#endif
}

struct wordlist *wordlist_create(void)
{
	struct wordlist *ws = malloc(sizeof *ws);
	TAILQ_INIT(ws);
	_wordlist_invariant(ws);
	return ws;
}

/* TODO: do unicode word break detection with ICU */
struct wordlist *wordlist_append(struct wordlist *ws, const char *text)
{
	const char *start;

	while (start = text, *text)
	{
		struct wordlist_entry *w = malloc(sizeof *w);
		w->start = text;
		while (!isspace(*text) && *text)
			text++;
		w->len = text - start;
		while (isspace(*text))
			text++;
		TAILQ_INSERT_TAIL(ws, w, entries);
	}
	_wordlist_invariant(ws);
	return ws;
}

/* note: underlying string must be freed by caller */
void wordlist_free(struct wordlist *ws)
{
	_wordlist_invariant(ws);
	while (!TAILQ_EMPTY(ws))
	{
		struct wordlist_entry *w = TAILQ_FIRST(ws);
		TAILQ_REMOVE(ws, w, entries);
		free(w);
	}
	free(ws);
}

/* TODO: it's greedy, use a better algo */
size_t print_wrapped(
		const struct wordlist *ws, const char *overhang,
		size_t width, bool flowed)
{
	struct wordlist_entry *w;
	_wordlist_invariant(ws);
	size_t total, longest_line = 0;

	width -= strlen(overhang);

	w = TAILQ_FIRST(ws);
	while (w)
	{
		for (total = 0;
			w && total + w->len + 1 < width;
			total += w->len + 1, w = TAILQ_NEXT(w, entries))
		{
			if (total > 0)
				putchar(' ');
			longest_line = MAX(total + w->len, longest_line);
			printf("%.*s", (int)w->len, w->start);
		}
		if (w)
		{
			/* extra space is for format=flowed */
			printf("%s\n%s", flowed ? " " : "", overhang);
		}
		else
			printf("\n");
	}
	return longest_line;
}
