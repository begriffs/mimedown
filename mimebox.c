#include <stdio.h>
#include <stdlib.h>

#include <cmark.h>

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

	return EXIT_SUCCESS;
}
