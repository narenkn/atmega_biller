.PHONY:

CFLAGS = -g -I. -I.. -DUNIT_TEST -DDEBUG $(CMDLINE_CFLAGS)
CC = gcc
VPATH = .:..

SOURCES = $(wildcard test_*.c)
TARGETS = $(subst .c, , $(SOURCES))

$(TARGETS):

test_% : test_%.c %.c
