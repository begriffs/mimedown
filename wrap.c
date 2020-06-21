#include "wrap.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

static void _wordlist_invariant(const struct wordlist *ws)
{
	assert(ws);
#ifndef NDEBUG
	struct wordlist_entry *w;
	TAILQ_FOREACH(w, ws, entries)
		assert(w->len >= 0);
#endif
}

/* TODO: do unicode word break detection with ICU */
struct wordlist *build_wordlist(const char *text)
{
	struct wordlist *ws = malloc(sizeof *ws);
	const char *start;
	TAILQ_INIT(ws);

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
void free_wordlist(struct wordlist *ws)
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
void print_wrapped(const struct wordlist *ws, const int width)
{
	int total = 0;
	struct wordlist_entry *scout, *safe;
	_wordlist_invariant(ws);

	safe = scout = TAILQ_FIRST(ws);
	while (scout != NULL)
	{
		for (total = 0, scout = safe;
			scout && total + scout->len < width;
			total += scout->len, scout = TAILQ_NEXT(scout, entries))
		{
			printf("%.*s ", (int)scout->len, scout->start);
			safe = scout;
		}
		printf("\n");
	}
}
