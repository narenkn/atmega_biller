.PHONY: headers clean make_tests run_tests

NCURSES := /tool/pandora/.package/ncurses-5.7
CFLAGS  := -g -I. -I.. -DUNIT_TEST -DDEBUG $(CMDLINE_CFLAGS) -I$(NCURSES)/include -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
CFLAGS += -DLCD_USE_FUNCTIONS=1 -DLCD_DPORT_SIZE=4 -DBUFSS_SIZE=336 -DDS1307 -DITEM_SUBIDX_NAME=4 -D__AVR_ATmega32__=1
CFLAGS += -DMENU_USER_ENABLE=0 -DMENU_SETTING_ENABLE=1 -DMENU_SDSAVE_EN=0 -DMENU_DELBILL=1 -DMENU_ITEM_FUNC=1 -DMENU_DIAG_FUNC=1 -DFF_ENABLE=0 -DUNICODE_ENABLE=0
LDFLAGS := -L$(NCURSES)/lib -lncurses
CC = gcc
VPATH = .:..

SOURCE = $(wildcard test_*.c)
SOURCE_EXCLUDES = test_common # not a test
SOURCE_EXCLUDES	+= test_menu_i test_menu_setting_i # interactive
SOURCE_EXCLUDES += test_menu_3 # users, passwd
TESTS =  $(filter-out $(SOURCE_EXCLUDES), $(subst .c, $(EXEEXT), $(SOURCE)))
TESTSEXE = $(TESTS:.c=)

run_tests: make_tests
	for test in $(TESTSEXE) ; do echo -n $$test ; ./$$test ; done

make_tests: $(TESTSEXE) ../ep_ds.h ../version.h

include ../make.proj

headers : ../ep_ds.h ../version.h

% : %.c headers
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

%.dat : %.csv
	python ../utils/csv2dat --to_dat --in_file $< --out_file $@

clean:
	-\rm -f core.* *~ $(TESTS)
