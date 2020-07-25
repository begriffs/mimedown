.POSIX:

OBJS = filetype.o wrap.o smtp.o
CFLAGS = -std=c99 -g -pedantic -Wall -Wextra -Wshadow -D_POSIX_C_SOURCE=200112L
LIBCOMPAT = compat/libcompat.a

.SUFFIXES :
.SUFFIXES : .o .c

include config.mk

md2mime : md2mime.c $(OBJS) $(LIBCOMPAT)
	$(CC) $(CFLAGS) $(LDFLAGS) -Lcompat -o $@ md2mime.c $(OBJS) $(LDLIBS) -lcompat

wrap.o : wrap.c wrap.h vendor/queue.h

smtp.o : smtp.c smtp.h

filetype.o : filetype.c filetype.h

${LIBCOMPAT} :
	( cd compat && $(MAKE) )

clean :
	rm -f md2mime *.o
	( cd compat && $(MAKE) clean )
