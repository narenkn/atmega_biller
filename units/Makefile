.PHONY:

NCURSES := /tool/pandora/.package/ncurses-5.7
CFLAGS  := -g -I. -I.. -DUNIT_TEST -DDEBUG $(CMDLINE_CFLAGS) -I$(NCURSES)/include
LDFLAGS := -L$(NCURSES)/lib -lncurses
CC = gcc
VPATH = .:..

SOURCES = $(wildcard test_*.c)
TARGETS = $(subst .c, , $(SOURCES))

$(TARGETS):

test_% : test_%.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

clean:
	-\rm -f core.* $(TARGETS)
