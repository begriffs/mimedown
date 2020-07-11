.POSIX:

OBJS = filetype.o wrap.o

CFLAGS = -std=c99 -g -pedantic -Wall -Wextra -Wshadow

.SUFFIXES :
.SUFFIXES : .o .c

include config.mk

md2mime : md2mime.c $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ md2mime.c $(OBJS) $(LDLIBS)

wrap.o : wrap.c wrap.h vendor/queue.h

filetype.o : filetype.c filetype.h

clean :
	rm -f md2mime *.o
