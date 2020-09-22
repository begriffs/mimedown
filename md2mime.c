#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include "smtp.h"
#include "wrap.h"
#include "vendor/uthash.h"

/* libcmark */
#include <cmark.h>

#define TEXTWIDTH 72

struct doc_link
{
	size_t id;
	char pretty_id[10];
	struct wordlist *caption;
	const char *url;
	UT_hash_handle hh;
} *g_links = NULL;

void snarf_node(cmark_iter *iter)
{
	cmark_event_type ev_type;
	cmark_node *cur = cmark_iter_get_node(iter);
	cmark_node_type outer = cmark_node_get_type(cur);

	while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE)
	{
		if (ev_type == CMARK_EVENT_EXIT &&
		    cmark_node_get_type(cmark_iter_get_node(iter)) == outer)
			break;
	}
}

struct doc_link *parse_link(cmark_iter *iter)
{
	cmark_event_type ev_type;
	cmark_node *cur = cmark_iter_get_node(iter);

	struct doc_link *link = NULL;
	const char *url;

	assert(cmark_node_get_type(cur) == CMARK_NODE_LINK);

	url = cmark_node_get_url(cur);
	HASH_FIND_STR(g_links, url, link);
	if (link)
	{	/* seen it, move on */
		snarf_node(iter);
		return link;
	}
	link = malloc(sizeof *link);
	link->id = HASH_COUNT(g_links);
	snprintf(link->pretty_id, sizeof link->pretty_id,
			"[%zu]", link->id);
	link->url = url;
	link->caption = wordlist_create();
	HASH_ADD_KEYPTR(hh, g_links, link->url,
			strlen(link->url), link);

	while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE)
	{
		cur = cmark_iter_get_node(iter);
		cmark_node_type type = cmark_node_get_type(cur);

		if (ev_type == CMARK_EVENT_EXIT && type == CMARK_NODE_LINK)
			break;
		if (ev_type == CMARK_EVENT_ENTER && type == CMARK_NODE_TEXT)
			wordlist_segment(link->caption, cmark_node_get_literal(cur));
	}
	return link;
}

size_t render_inner_text(cmark_iter *iter, char *prefix, char *overhang, bool flow)
{
	cmark_event_type ev_type;
	cmark_node *cur = cmark_iter_get_node(iter);
	cmark_node_type outer_type = cmark_node_get_type(cur);
	struct wordlist *ws = wordlist_create();
	size_t ret;

	while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE)
	{
		cur = cmark_iter_get_node(iter);
		cmark_node_type type = cmark_node_get_type(cur);

		if (ev_type == CMARK_EVENT_EXIT && type == outer_type)
			break;
		if (ev_type == CMARK_EVENT_ENTER)
		{
			if (type == CMARK_NODE_LINK)
			{
				struct doc_link *link = parse_link(iter);
				if (link)
				{
					wordlist_concat(ws, link->caption);
					wordlist_segment(ws, link->pretty_id);
				}
				else
					wordlist_segment(ws, "[could not parse link]");
			}
			else
			{
				const char *content = cmark_node_get_literal(cur);
				if (content)
					wordlist_segment(ws, content);
			}
		}
	}

	printf("%s", prefix);
	ret = print_wrapped(ws, overhang, TEXTWIDTH, flow);
	wordlist_free(ws);
	return ret;
}

void render_list(cmark_iter *iter)
{
	cmark_event_type ev_type;
	cmark_node *cur = cmark_iter_get_node(iter);
	cmark_list_type t = cmark_node_get_list_type(cur);
	char *pad = ((t == CMARK_ORDERED_LIST) ? "   " : "  "),
		 marker[10] = "* ";
	int i = 1;

	assert(cmark_node_get_type(cur) == CMARK_NODE_LIST);

	while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE)
	{
		cur = cmark_iter_get_node(iter);
		cmark_node_type type = cmark_node_get_type(cur);

		if (ev_type == CMARK_EVENT_EXIT && type == CMARK_NODE_LIST)
			break;
		if (ev_type == CMARK_EVENT_ENTER && type == CMARK_NODE_ITEM)
		{
			if (t == CMARK_ORDERED_LIST)
				snprintf(marker, sizeof marker, "%d. ", i++);
			render_inner_text(iter, marker, pad, false);
		}
	}
	puts("");
}

void render_heading(cmark_iter *iter)
{
	cmark_node *cur = cmark_iter_get_node(iter);
	char borders[] = {'~', '#', '=', '-', '~'}, b;
	size_t width;
	unsigned level;

	assert(cmark_node_get_type(cur) == CMARK_NODE_HEADING);

	level = cmark_node_get_heading_level(cur);
	b = (level > sizeof(borders)-1)
		? borders[sizeof(borders)-1]
		: borders[level];
	width = render_inner_text(iter, "", "", true);
	while (width-- > 0)
		putchar(b);
	puts("\n");
}

int main(int argc, char **argv)
{
	char buf[BUFSIZ];
	size_t bytes;
	cmark_node *doc;

	int c;
	bool arg_err = false, preserve_headers = false;
	char *host;
	while ((c = getopt(argc, argv, "p")) != -1)
	{
		switch(c)
		{
			case 'p':
				preserve_headers = true;
				break;
			case '?':
				fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
				arg_err = true;
				break;
			default:
				assert(0);
		}
	}
	if (arg_err || optind >= argc)
	{
		fprintf(stderr,
				"usage: %s [-p] sender-host\n"
				"\twhere -p = preserve headers\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (optind < argc)
	{
		host = argv[optind];
		if (strchr(host, '@'))
		{
			fputs("Hostname should not contain '@'\n", stderr);
			return EXIT_FAILURE;
		}
	}

	if (preserve_headers)
	{
		unsigned n = 0;
		for (c = 0; (c = getchar()) != EOF; )
		{
			if (c == '\n')
			{
				if (++n > 1)
					break; /* two consecutive newlines */
			}
			else
				n = 0;
			putchar(c);
		}
	}

	cmark_parser *p = cmark_parser_new(CMARK_OPT_DEFAULT);
	while ((bytes = fread(buf, 1, sizeof(buf), stdin)) > 0)
	{
		cmark_parser_feed(p, buf, bytes);
		if (bytes < sizeof(buf))
			break;
	}
	doc = cmark_parser_finish(p);
	cmark_parser_free(p);

	cmark_event_type ev_type;
	cmark_iter *iter = cmark_iter_new(doc);

	enum section_type
	{
		SEC_NONE, SEC_MSG, SEC_CODE
	} section, prev_section = SEC_NONE;

	char *msgid = generate_msgid(host),
	     *boundary_a = generate_mime_boundary(),
	     *boundary_b = generate_mime_boundary();
	printf("Message-ID: <%s>\n", msgid);
	printf("MIME-Version: 1.0\n"
	       "Content-Type: multipart/alternative; boundary=\"%s\"\n\n"
		   "--%s\n"
	       "Content-Type: multipart/mixed; boundary=\"%s\"\n",
		   boundary_a, boundary_a, boundary_b);

	int nth_code_block = 0, nth_block = 0;
	while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE)
	{
		const char *code;
		cmark_node *cur = cmark_iter_get_node(iter);
		cmark_node_type type = cmark_node_get_type(cur);
		if (ev_type == CMARK_EVENT_ENTER)
		{
			section = (type == CMARK_NODE_CODE_BLOCK)
				? SEC_CODE
				: SEC_MSG;
			if (section != prev_section)
			{
				prev_section = section;
				printf("\n--%s\n", boundary_b);
				if (section == SEC_CODE)
				{
					const char *filename = cmark_node_get_fence_info(cur);
					printf("Content-Type: %s; charset=\"utf-8\"\n",
						filename_mime(filename));
					if (*filename == '\0')
						printf(
							"Content-Disposition: inline; filename=code-%d.txt\n",
							nth_code_block++);
					else
						printf(
							"Content-Disposition: inline; filename=%s\n",
							filename);
				}
				else
				{
					puts("Content-Type: text/plain; charset=\"utf-8\"; "
					     "format=\"flowed\"");
					puts("Content-Disposition: inline");
				}
				printf("Content-ID: <%i.%s>\n\n", nth_block++, msgid);
			}
			switch (cmark_node_get_type(cur))
			{
				case CMARK_NODE_LIST:
					render_list(iter);
					break;
				case CMARK_NODE_HEADING:
					render_heading(iter);
					break;
				case CMARK_NODE_BLOCK_QUOTE:
					render_inner_text(iter, "> ", "> ", true);
					puts("");
					break;
				case CMARK_NODE_CODE_BLOCK:
					code = cmark_node_get_literal(cur);
					if (code)
						fputs(code, stdout);
					break;
				case CMARK_NODE_PARAGRAPH:
					render_inner_text(iter, "", "", true);
					puts("");
					break;
				default:
					break;
			}
		}
	}

	if (HASH_COUNT(g_links))
	{
		struct doc_link *x, *next;

		printf("\n--%s\n", boundary_b);
		puts("Content-Type: text/uri-list; charset=\"utf-8\"");
		puts("Content-Disposition: inline; filename=references.uri\n");

		/* that the ids are printed in sorted order relies on the fact
		 * that items are visited in insertion order */
		for (x = g_links; x; x = next)
		{
			printf("# %zu: ", x->id);
			print_wrapped(x->caption, "", SIZE_MAX, false);
			puts(x->url);
			next = x->hh.next;

			wordlist_free(x->caption);
			HASH_DEL(g_links, x);
			free(x);
		}
	}

	printf("\n--%s--\n\n",boundary_b);
	printf("--%s\n", boundary_a);
	puts("Content-Type: text/html; charset=\"utf-8\"");
	puts("Content-Disposition: inline\n");

	puts("<!DOCTYPE HTML PUBLIC \"ISO/IEC 15445:2000//DTD HTML//EN\">");
	puts("<html><head><title>Foo</title></head>");
	puts("<body>");

	char *html = cmark_render_html(doc, CMARK_OPT_DEFAULT);
	fputs(html, stdout);
	free(html);
	
	puts("</body></html>\n");
	printf("--%s--\n", boundary_a);
	cmark_iter_free(iter);
	cmark_node_free(doc);
	free(msgid);
	free(boundary_a);
	free(boundary_b);

	return EXIT_SUCCESS;
}
