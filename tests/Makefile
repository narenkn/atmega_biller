###############################################################################
# Makefile for the individual test programs
###############################################################################

.PHONY: clean size all headers

## General Flags
PROJECT = 
#MCU = atmega32
MCU = atmega1284p
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU) -I.. -DNO_MAIN -DLCD_USE_FUNCTIONS=1

## Compile options common for all C compilation units.
CFLAGS = $(COMMON) $(CLINE_ARG)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DF_CPU=8000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 
CFLAGS += -DBUFSS_SIZE=272 -DDS1307 -DITEM_SUBIDX_NAME=4
CFLAGS += -DMENU_USER_ENABLE=0           # 8 different Users, login
CFLAGS += -DMENU_SETTING_ENABLE=1        # Set menu items in device
CFLAGS += -DMENU_SDSAVE_EN=0             # Save/restore from/to SD
CFLAGS += -DMENU_DELBILL=1               # Delete bill
CFLAGS += -DMENU_ITEM_FUNC=1             # Add,Del item
CFLAGS += -DMENU_DIAG_FUNC=1             # Diagnosis utils
CFLAGS += -DFF_ENABLE=0                  # Enable SD file system
CFLAGS += -DMENU_BILL_STORE_ITEM_DESCR=0 # Store item with bill
CFLAGS += -DMENU_PRINTER_ENABLE=1        # Enable printing routines
CFLAGS += -DUNICODE_ENABLE=0             # Unicode in device

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
BOOTLOAD = 0x3800
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=$(SOURCE:.c=.map)
LDFLAGS += -Wl,--section-start=.bootloader=$(BOOTLOAD)

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## Files
SOURCE = $(wildcard test_*.c)
SOURCE_EXCLUDES = test_common
TARGETS =  $(filter-out $(SOURCE_EXCLUDES), $(subst .c, , $(SOURCE)))
TARGET = $(word 1, $(TARGETS))

## Objects that must be built in order to link
OBJECTS = $(SOURCE:.c=.o)

## Objects explicitly added by the user
LINKONLYOBJECTS =

## Build
all: $(TARGET).elf $(TARGET).hex $(TARGET).eep $(TARGET).lss size

##
include ../make.proj

##
headers: ../ep_ds.h ../version.h 

## Compile
VPATH := ..
%.o: %.c headers
	$(CC) $(INCLUDES) $(CFLAGS) -c -o $@ $<

##Link
%.elf: %.o
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $@

%.hex: %.elf
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: %.elf
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: %.elf
	avr-objdump -h -S $< > $@

size: $(TARGET).elf
	@echo
	@avr-size -C --mcu=$(MCU) $(TARGET).elf

## Clean target
clean:
	-$(RM) *.o *.elf dep/* *.hex *.eep *.lss *.map *~

## Other dependencies
-include $(shell mkdir dep 2>/dev/null)
