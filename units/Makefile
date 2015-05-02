.PHONY: headers clean make_tests run_tests

NCURSES := /tool/pandora/.package/ncurses-5.7
CFLAGS  := -g -I. -I.. -DUNIT_TEST -DDEBUG $(CMDLINE_CFLAGS) -I$(NCURSES)/include -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
CFLAGS += -DLCD_USE_FUNCTIONS=1 -DLCD_DPORT_SIZE=4 -DBUFSS_SIZE=272 -DDS1307 -DITEM_SUBIDX_NAME=4 -D__AVR_ATmega32__=1
CFLAGS += -DMENU_USER_ENABLE=1 -DMENU_SETTING_ENABLE=1 -DMENU_SDSAVE_EN=1 -DMENU_DELBILL=1 -DMENU_ITEM_FUNC=1 -DMENU_DIAG_FUNC=1 -DFF_ENABLE=1
LDFLAGS := -L$(NCURSES)/lib -lncurses
CC = gcc
VPATH = .:..

SOURCE = $(wildcard test_*.c)
SOURCE_EXCLUDES = test_common test_menu_i test_menu_setting_i
TESTS =  $(filter-out $(SOURCE_EXCLUDES), $(subst .c, $(EXEEXT), $(SOURCE)))

make_tests: $(TESTS) ../ep_ds.h ../version.h

run_tests: make_tests
	for test in $(TESTS) ; do echo -n $$test ; ./$$test ; done

include ../make.proj

headers : ../ep_ds.h ../version.h

% : %.c headers
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

%.dat : %.csv
	python ../utils/csv2dat --to_dat --in_file $< --out_file $@

clean:
	-\rm -f core.* *~ $(TESTS)
