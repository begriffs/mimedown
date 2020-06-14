#include <stdio.h>
#include <stdlib.h>

#include <cmark.h>

int main(void)
{
	char buf[BUFSIZ];
	size_t bytes;
	int depth = 0;
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

	while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE)
	{
		cmark_node *cur = cmark_iter_get_node(iter);
		cmark_node_type type = cmark_node_get_type(cur);
		for(int i = 0; i < depth; i++)
			putchar(' ');
		switch (ev_type)
		{
			case CMARK_EVENT_NONE:
				printf("?");
				break;
			case CMARK_EVENT_DONE:
				printf(".");
				break;
			case CMARK_EVENT_ENTER:
				printf(">");
				if (type != CMARK_NODE_TEXT && type != CMARK_NODE_CODE_BLOCK &&
						type != CMARK_NODE_SOFTBREAK)
					depth++;
				break;
			case CMARK_EVENT_EXIT:
				printf("<");
				depth--;
				break;
			default:
				abort();
		}
		printf(" %s\n", cmark_node_get_type_string(cur));
	}
	cmark_iter_free(iter);

	return EXIT_SUCCESS;
}
