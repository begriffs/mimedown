#define _GNU_SOURCE
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef HAVE_GETRANDOM
#include <sys/random.h>
#endif

#include "smtp.h"

static void get_random_bytes(void *buf, size_t n)
{
#if defined HAVE_ARC4RANDOM  /* BSD */
	arc4random_buf(buf, n);
#elif defined HAVE_GETRANDOM /* Linux, GLIBC >= 2.25 */
	getrandom(buf, n, 0);
#elif defined __linux__ && defined __GLIBC__ && \
    __GLIBC__ <= 2 &&  __GLIBC_MINOR__ < 25 /* Linux, GLIBC < 2.25 */
    #include <unistd.h>
    #include <sys/syscall.h>
    syscall(SYS_getrandom, buf, n, 0);
#else
#error OS does not provide recognized function to get entropy
#endif
}

/* Print number using symbols from an alphabet. The symbols are written
 * little-endian, but the output suffices for getting unique strings */
static char *alpha_radix(uintmax_t n, const char *alpha, size_t nalpha)
{
	char *p, *ret = malloc(1 + (size_t)ceil(log(n)/log(nalpha)));
	if (!(p = ret))
		return NULL;
	do
	{
		*p++ = alpha[n % nalpha];
		n /= nalpha;
	} while(n > 0);
	*p = '\0';
	return ret;
}

/* caller must free */
char *generate_msgid(const char *host)
{
	/* atext tokens from RFC 2822
	 * https://tools.ietf.org/html/rfc2822.html#section-3.2.4
	 * However, to allow referencing msg ids in urls, avoid:
	 *   URL reserved chars ; / ? : @ = &
	 *   URL unsafe chars " < > # % { } | \ ^ ~ [ ] ` */
	static const char atext[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"$!*-~'+_";

	uint_least64_t rnd;
	get_random_bytes(&rnd, sizeof rnd);

	/* Typically time() returns seconds since the UNIX epoch. The C99 standard
	 * says that the range and precision is implementation-defined. What
	 * matters is getting numbers that don't repeat for a long time. */
	char *s = alpha_radix(time(NULL), atext, (sizeof atext)-1),
	     *t = alpha_radix(rnd, atext, (sizeof atext)-1),
	     *msgid = NULL;
	if (!s || !t)
		goto done;

	msgid = malloc(strlen(s) + strlen(t) + strlen(host) + 3);
	if (msgid)
		sprintf(msgid, "%s.%s@%s", s, t, host);
done:
	if (s) free(s);
	if (t) free(t);
	return msgid;
}

/* caller must free */
char *generate_mime_boundary(void)
{
	/* https://tools.ietf.org/html/rfc2046#section-5.1.1 */
	static const char bcharsnospace[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"'()+_,-./:=?";
	uintmax_t rnd;
	get_random_bytes(&rnd, sizeof rnd);
	return alpha_radix(rnd, bcharsnospace, (sizeof bcharsnospace)-1);
}

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
