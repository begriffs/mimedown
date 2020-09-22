#include "wrap.h"

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(x,y) ((x)<(y) ? (y) : (x))

static void _wordlist_invariant(const struct wordlist *ws)
{
	assert(ws);
	assert(ws->len <= ws->_alloced);
#ifndef NDEBUG
	for (size_t i = 0; i < ws->len; i++)
	{
		struct wordlist_entry *w = ws->words+i;
		assert(w->len > 0);
		assert(strcspn(w->start, " \t") >= (size_t)w->len);
	}
#endif
}

struct wordlist *wordlist_create(void)
{
	struct wordlist *ws = malloc(sizeof *ws);
	if (!ws) abort();
	ws->words = NULL;
	ws->_alloced = ws->len = 0;
	_wordlist_invariant(ws);
	return ws;
}

/* underlying string memory is shared by copies */
void wordlist_append_copy(struct wordlist *ws, const struct wordlist_entry *w)
{
	_wordlist_invariant(ws);
	if (ws->len >= ws->_alloced)
	{
		ws->_alloced = ws->_alloced == 0 ? 256 : ws->_alloced * 2;
		ws->words = realloc(ws->words, ws->_alloced * sizeof(*w));
		if (!ws->words) abort();
	}
	ws->words[ws->len++] = *w;
	_wordlist_invariant(ws);
}

/* TODO: do unicode word break detection with ICU */
struct wordlist *wordlist_segment(struct wordlist *ws, const char *text)
{
	struct wordlist_entry w;
	_wordlist_invariant(ws);
	while (*text)
	{
		while (isspace(*text))
			text++;
		w.start = text;
		while (*text && !isspace(*text))
			text++;
		w.len = text - w.start;
		if (w.len > 0)
			wordlist_append_copy(ws, &w);
		while (isspace(*text))
			text++;
	}
	_wordlist_invariant(ws);
	return ws;
}

/* src and dst entries will share the underlying string memory */
void wordlist_concat(struct wordlist *dst, const struct wordlist *src)
{
	_wordlist_invariant(src);
	_wordlist_invariant(dst);

	for (size_t i = 0; i < src->len; i++)
		wordlist_append_copy(dst, src->words+i);

	_wordlist_invariant(dst);
}

/* note: underlying string must be freed by caller */
void wordlist_free(struct wordlist *ws)
{
	_wordlist_invariant(ws);
	free(ws->words);
	free(ws);
}


static float **line_costs(const struct wordlist *ws, unsigned width)
{
	size_t i, j;
	/* underlying chunk of memory that can be freed in one go */
	float *mem = malloc((sizeof *mem) * ws->len * ws->len);
	if (!mem) abort();
	for (i = 0; i < ws->len * ws->len; i++)
		mem[i] = INFINITY;

	/* split the memory into rows and columns */
	float **table = malloc((sizeof *table) * ws->len);
	if (!table) abort();
	for (i = 0; i < ws->len; i++)
		table[i] = mem + (i * ws->len);

	for (i = 0; i < ws->len; i++)
	{
		unsigned linelen = 0;
		for (j = i; j < ws->len; j++)
		{
			linelen += (unsigned)ws->words[j].len;
			if (linelen <= width)
				table[i][j] = pow(width - linelen, 2);
			else
				break;
			linelen++; /* space between words */
		}
	}

	return table;
}

size_t *best_breaks(const struct wordlist *ws, unsigned width)
{
	size_t i, j;
	float **costs    = line_costs(ws, width);
	float  *min_cost = malloc(1 + ws->len * sizeof *min_cost);
	size_t *next_brk = malloc(ws->len * sizeof *next_brk);
	if (!costs || !min_cost || !next_brk) abort();

	for (i = ws->len-1; i < ws->len; i--)
		min_cost[i] = INFINITY;
	min_cost[ws->len] = 0; /* fake item past the end */

	/* below 0, unsigned wraps above ws->len */
	for (i = ws->len-1; i < ws->len; i--)
		for (j = ws->len-1; i <= j && j < ws->len; j--)
			if (costs[i][j] + min_cost[j+1] <= min_cost[i])
			{
				min_cost[i] = costs[i][j] + min_cost[j+1];
				next_brk[i] = j+1;
			}

	free(*costs);
	free(costs);
	free(min_cost);
	return next_brk;
}

size_t print_wrapped(
		const struct wordlist *ws, const char *overhang,
		size_t width, bool flowed)
{
	_wordlist_invariant(ws);
	width -= strlen(overhang);

	size_t i, brk;
	unsigned total = 0, longest_line = 0;
	size_t *brks = best_breaks(ws, width);

	for (i = 0, brk = *brks; i < ws->len; i++)
	{
		if (i == brk)
		{
			total = 0;
			brk = brks[i];
			if (i < ws->len)
				printf("%s\n%s", flowed ? " " : "", overhang);
			else
				putchar('\n');
		}
		if (total > 0)
			putchar(' ');
		longest_line = MAX(total + ws->words[i].len, longest_line);
		printf("%.*s", (int)ws->words[i].len, ws->words[i].start);

		total += ws->words[i].len + 1;
	}
	putchar('\n');
	free(brks);

	return longest_line;
}
