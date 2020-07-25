#include <stdio.h>
#include <stdlib.h>
#include "shim.h"

void arc4random_buf(void *buf, size_t nbytes)
{
	FILE *fp = fopen("/dev/urandom", "rb");
	if (!fp)
	{
		fputs("arc4random_buf shim: cannot open /dev/urandom\n", stderr);
		abort();
	}

	if (fread(buf, 1, nbytes, fp) != nbytes)
	{
		fputs("arc4random_buf shim: too few bytes in /dev/urandom\n", stderr);
		fclose(fp);
		abort();
	}
	fclose(fp);
}
