.POSIX:

CFLAGS = -std=c99 -g -pedantic -Wall -Wextra -Wshadow -Wno-missing-braces -D_POSIX_C_SOURCE=200112L -g

.SUFFIXES :
.SUFFIXES : .o .c

include config.mk

mimebox : mimebox.c

clean :
	rm -f mimebox *.o
