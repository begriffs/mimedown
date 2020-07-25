.POSIX:

CFLAGS = -std=c99 -g -pedantic -Wall -Wextra -Wshadow -D_POSIX_C_SOURCE=200112L
COMPAT = arc4random.o

libcompat.a : $(COMPAT)
	$(AR) $(ARFLAGS) $@ $(COMPAT)

$(COMPAT) : shim.h

.SUFFIXES :
.SUFFIXES : .o .c

.c.o:
	if $(CC) $(CLFAGS) -Werror -c test-$< >/dev/null 2>&1; then $(CC) $(CFLAGS) -c -o $@ empty.c; else $(CC) $(CFLAGS) -c -o $@ $<; fi

clean:
	rm *.[ao]
