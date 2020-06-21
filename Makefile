.POSIX:

CFLAGS = -std=c99 -g -pedantic -Wall -Wextra -Wshadow -Wno-missing-braces -D_POSIX_C_SOURCE=200112L -g

.SUFFIXES :
.SUFFIXES : .o .c

include config.mk

md2mime : md2mime.c wrap.o

wrap.o : wrap.c wrap.h vendor/queue.h

clean :
	rm -f md2mime *.o
