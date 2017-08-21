.PHONY: headers clean make_tests run_tests

NCURSES := /tool/pandora/.package/ncurses-5.7
NCURSES_INC := $(NCURSES)/include
NCURSES_LIB := $(NCURSES)/lib
#NCURSES_INC := $(shell ncurses6-config --includedir)
#NCURSES_LIB := $(shell ncurses6-config --libdir)
CFLAGS  := -g -I. -I.. -DUNIT_TEST=1 -DDEBUG=0 $(CMDLINE_CFLAGS) -I$(NCURSES_INC) -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
CFLAGS += -DLCD_USE_FUNCTIONS=1 -DBUFSS_SIZE=1056 -DDS1307=0 -D__AVR_ATmega32__=1  -std=c99
CFLAGS += -DMENU_USER_ENABLE=1 -DMENU_SETTING_ENABLE=1 -DMENU_SDSAVE_EN=1 -DMENU_DELBILL=1 -DMENU_ITEM_FUNC=1 -DMENU_DIAG_FUNC=1 -DFF_ENABLE=1 -DUNICODE_ENABLE=0 -DMENU_PRINTER_ENABLE=1
CFLAGS += -DNVFLASH_EN=1                 # Non Volatile Flash
LDFLAGS := -L$(NCURSES_LIB) -lncurses
CC = gcc
VPATH = .:..

SOURCE = $(wildcard test_*.c)
SOURCE_EXCLUDES = test_sdsettings.c test_ff_features.c test_menushowbill.c
TESTS =  $(filter-out $(SOURCE_EXCLUDES), $(SOURCE))
TESTSEXE = $(TESTS:.c=)

run_tests: make_tests
	@echo '------------------------'
	@if [ -d dir1 ] ; then rm -rf dir1 ; fi
	@for test in $(TESTSEXE) ; do echo -n $$test ": " ; ./$$test 2>&1 | grep -v '^seed' | wc -l > nu ; if [ "0" = `cat nu` ] ; then echo PASSED ; else echo FAILED ; fi ; done

make_tests: $(TESTSEXE) ../ep_ds.h ../version.h

include ../make.proj

headers : ../ep_ds.h ../version.h

% : %.c headers
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

%.dat : %.csv
	python ../utils/csv2dat --to_dat --in_file $< --out_file $@

clean:
	-\rm -rf core.* *~ $(TESTSEXE) *.exe *.stackdump dir1
