.PHONY: headers clean

NCURSES := /tool/pandora/.package/ncurses-5.7
CFLAGS  := -g -I. -I.. -DUNIT_TEST -DDEBUG $(CMDLINE_CFLAGS) -I$(NCURSES)/include
CFLAGS += -DLCD_USE_FUNCTIONS=1 -DLCD_DPORT_SIZE=4 -DBUFSS_SIZE=272 -DDS1307 -DITEM_SUBIDX_NAME=4 -DATMega32=1
CFLAGS += -DMENU_USER_ENABLE=1 -DMENU_SETTING_ENABLE=1 -DMENU_SDSAVE_EN=1 -DMENU_DELBILL=1 -DMENU_ITEM_FUNC=1 -DMENU_DIAG_FUNC=1 -DFF_ENABLE=1
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

%.dat : %.csv
	python ../utils/csv2dat --to_dat --in_file $< --out_file $@

clean:
	-\rm -f core.* $(TARGETS)
