#include <stdio.h>
#include <stdlib.h>

#include "wrap.h"

/* libcmark */
#include <cmark.h>

#define TEXTWIDTH 72

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
					puts("Content-Type: text/x-c");
					printf("Content-Disposition: inline; filename=%d.c;\n\n",
					       nth_code_block++);
				}
				else
				{
					puts("Content-Type: text/plain");
					puts("Content-Disposition: inline\n");
				}
			}
			const char *content = cmark_node_get_literal(cur);
			if (content)
			{
				struct wordlist *ws = build_wordlist(content);
				print_wrapped(ws, TEXTWIDTH);
				free_wordlist(ws);
			}
		}
	}
	cmark_iter_free(iter);
	cmark_node_free(doc);

	return EXIT_SUCCESS;
}
