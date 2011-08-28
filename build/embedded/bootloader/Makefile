###############################################################################
# Makefile for the project bootloader
###############################################################################

## General Flags
PROJECT = bootloader
MCU = atmega8
TARGET = $(PROJECT).elf
CC = avr-gcc
SRCDIR = ../../../src/embedded/$(PROJECT)

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

YACA_PATH = $(shell cat /etc/yaca.path)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -DF_CPU=2000000UL -Os -fsigned-char -I$(YACA_PATH)/build/include
CFLAGS += -MD -MP -MT $(*F).o

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -L../lib
LIBS = -lyaca-bl


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = $(PROJECT).o utils.o trampolines.o

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) $(PROJECT).hex $(PROJECT).eep $(PROJECT).lss size

## Compile

%.o: $(SRCDIR)/%.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) -o $(TARGET) -Wl,-T linkerscript $(LIBS)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size ${TARGET}

load: $(TARGET).hex
	avrdude -q -u -C /usr/local/etc/avrdude.conf -p m8 -c usbasp -U flash:w:bootloader.hex:a -v

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) $(PROJECT).elf $(PROJECT).hex $(PROJECT).eep $(PROJECT).lss *.d

## Other dependencies
#-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

