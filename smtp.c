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
#elif defined HAVE_GETRANDOM /* Linux */
	getrandom(buf, n, 0);
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
