#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "wrap.h"

/* libcmark */
#include <cmark.h>

#define TEXTWIDTH 72

size_t render_inner_text(cmark_iter *iter, char *prefix, char *overhang)
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
			const char *content = cmark_node_get_literal(cur);
			if (content)
				wordlist_append(ws, content);
		}
	}

	printf("%s", prefix);
	ret = print_wrapped(ws, overhang, TEXTWIDTH);
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
			render_inner_text(iter, marker, pad);
		}
	}
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
	width = render_inner_text(iter, "", "");
	while (width-- > 0)
		putchar(b);
	puts("\n");
}

int main(void)
{
	char buf[BUFSIZ];
	size_t bytes;
	cmark_node *doc;

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

	puts("MIME-Version: 1.0\n"
	     "Content-Type: multipart/alternative; boundary=boundary41\n\n"
		 "--boundary41\n"
	     "Content-Type: multipart/mixed; boundary=boundary42");

	while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE)
	{
		int nth_code_block = 0;
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
				puts("\n--boundary42");
				if (section == SEC_CODE)
				{
					puts("Content-Type: text/x-c; charset=\"utf-8\"");
					printf("Content-Disposition: inline; filename=%d.c\n\n",
					       nth_code_block++);
				}
				else
				{
					puts("Content-Type: text/plain; charset=\"utf-8\"; "
					     "format=\"flowed\"");
					puts("Content-Disposition: inline\n");
				}
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
					render_inner_text(iter, "> ", "> ");
					break;
				case CMARK_NODE_PARAGRAPH:
					render_inner_text(iter, "", "");
					break;
				default:
					break;
			}
		}
	}
	puts("\n--boundary42--\n");
	puts("--boundary41");
	puts("Content-Type: text/html; charset=\"utf-8\"");
	puts("Content-Disposition: inline\n");

	puts("<!DOCTYPE HTML PUBLIC \"ISO/IEC 15445:2000//DTD HTML//EN\">");
	puts("<html><head><title>Foo</title></head>");
	puts("<body>");

	char *html = cmark_render_html(doc, CMARK_OPT_DEFAULT);
	fputs(html, stdout);
	free(html);
	
	puts("</body></html>\n");
	puts("--boundary41--");
	cmark_iter_free(iter);
	cmark_node_free(doc);

	return EXIT_SUCCESS;
}
