###############################################################################
# Makefile for the individual test programs
###############################################################################

.PHONY: clean size all headers

## General Flags
PROJECT = 
MCU = atmega128
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU) -I.. -DNO_MAIN -DLCD_USE_FUNCTIONS=1

## Compile options common for all C compilation units.
CFLAGS = $(COMMON) $(CLINE_ARG)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DF_CPU=8000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
BUILD_FLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 
BUILD_FLAGS += -DBUFSS_SIZE=1056 -DDS1307=0
BUILD_FLAGS += -DMENU_USER_ENABLE=1           # 8 different Users, login
BUILD_FLAGS += -DMENU_SETTING_ENABLE=1        # Set menu items in device
BUILD_FLAGS += -DMENU_SDSAVE_EN=1             # Save/restore from/to SD
BUILD_FLAGS += -DMENU_DELBILL=1               # Delete bill
BUILD_FLAGS += -DMENU_ITEM_FUNC=1             # Add,Del item
BUILD_FLAGS += -DMENU_DIAG_FUNC=1             # Diagnosis utils
BUILD_FLAGS += -DFF_ENABLE=1                  # Enable SD file system
BUILD_FLAGS += -DMENU_BILL_STORE_ITEM_DESCR=0 # Store item with bill
BUILD_FLAGS += -DMENU_PRINTER_ENABLE=1        # Enable printing routines
BUILD_FLAGS += -DUNICODE_ENABLE=0             # Unicode in device
BUILD_FLAGS += -DNVFLASH_EN=1                 # Non Volatile Flash

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
BOOTLOAD = 0xFE00
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=$(TARGET:.c=.map)
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

CFLAGS += $(BUILD_FLAGS)

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
