###############################################################################
# Makefile for the individual test programs
###############################################################################

.PHONY: clean all headers

## General Flags
PROJECT = 
#MCU = atmega32
MCU = atmega1284p
SOURCE := buzzer.c
TARGET = $(SOURCE:.c=)
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU) -I.. -DNO_MAIN -DLCD_USE_FUNCTIONS=1

## Compile options common for all C compilation units.
CFLAGS = $(COMMON) $(CLINE_ARG)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DF_CPU=8000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 
CFLAGS += -DBUFSS_SIZE=272 -DDS1307 -DITEM_SUBIDX_NAME=4
CFLAGS += -DMENU_USER_ENABLE=1 -DMENU_SETTING_ENABLE=1 -DMENU_SDSAVE_EN=1 -DMENU_DELBILL=1 -DMENU_ITEM_FUNC=1 -DMENU_DIAG_FUNC=1 -DFF_ENABLE=1

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
$(TARGET).elf: $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET).elf

%.hex: $(TARGET).elf
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET).elf
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET).elf
	avr-objdump -h -S $< > $@

size: $(TARGET).elf
	@echo
	@avr-size -C --mcu=$(MCU) $(TARGET).elf

## Clean target
clean:
	-$(RM) *.o *.elf dep/* *.hex *.eep *.lss *.map *~


## Other dependencies
##-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)
