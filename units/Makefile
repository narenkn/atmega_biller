.PHONY: headers clean

NCURSES := /tool/pandora/.package/ncurses-5.7
CFLAGS  := -g -I. -I.. -DUNIT_TEST -DDEBUG $(CMDLINE_CFLAGS) -I$(NCURSES)/include
LDFLAGS := -L$(NCURSES)/lib -lncurses
CC = gcc
VPATH = .:..

SOURCES = $(wildcard test_*.c)
TARGETS = $(subst .c, , $(SOURCES))

include ../make.proj

$(TARGETS):

headers : ../ep_ds.h ../version.h

% : %.c headers
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

clean:
	-\rm -f core.* $(TARGETS)
