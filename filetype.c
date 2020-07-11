#include <string.h>

const char *filename_mime(const char *f)
{
	static struct {
		const char *ext;
		const char *mime;
	} known[] = {
		{"asm", "text/x-asm"}, {"c", "text/x-c"},
		{"cpp", "text/x-c"}, {"csh", "text/x-script.csh"},
		{"css", "text/css"}, {"csv", "text/csv"},
		{"diff", "text/x-diff"}, {"el", "text/x-script.elisp"},
		{"eps", "application/postscript"}, {"f", "text/x-fortran"},
		{"h", "text/x-c"}, {"htm", "text/html"},
		{"html", "text/html"}, {"jav", "text/x-java-source"},
		{"java", "text/x-java-source"}, {"js", "text/javascript"},
		{"json", "application/json"}, {"ksh", "text/x-script.ksh"},
		{"lisp", "text/x-script.lisp"}, {"lsp", "text/x-script.lisp"},
		{"m", "text/x-m"}, {"md", "text/markdown"},
		{"p", "text/pascal"}, {"pas", "text/pascal"},
		{"patch", "text/x-patch"}, {"pl", "text/x-script.perl"},
		{"py", "text/x-script.python"}, {"s", "text/x-asm"},
		{"scm", "text/x-script.scheme"}, {"sh", "application/x-sh"},
		{"svg", "image/svg+xml"}, {"tcl", "text/x-script.tcl"},
		{"tcsh", "text/x-script.tcsh"}, {"tex", "application/x-tex"},
		{"troff", "text/troff"}, {"tsv", "text/tab-separated-values"},
		{"xml", "text/xml"}, {"zsh", "text/x-script.zsh"}
	};
	if (!f || !(f = strrchr(f, '.')))
		return "text/plain";
	f++;
	for (size_t i = 0; i < sizeof(known)/sizeof(*known); i++)
		if (strcmp(known[i].ext, f) == 0)
			return known[i].mime;
	return "text/plain";
}
