###############################################################################
# Makefile for the project libyaca-bl
###############################################################################

## General Flags
PROJECT = libyaca-bl
MCU = atmega8
TARGET = libyaca-bl.a
CC = avr-gcc
SRCDIR = ../../../src/embedded/$(PROJECT)

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
F_CPU ?= 2000000UL
CLOCK_PRESCALER_MAGIC ?= MCP2515_CLKOUT_2MHZ
CFLAGS += -Wall -gdwarf-2 -DF_CPU=$(F_CPU) -DCLOCK_PRESCALER_MAGIC=$(CLOCK_PRESCALER_MAGIC) $(flags) -O2 -fsigned-char
CFLAGS += -MD -MP -MT $(*F).o

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += 


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = mcp2515.o yaca-bl.o spi.o

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET)
# size yaca-serial.hex yaca-serial.eep yaca-serial.lss size copy

## Compile

asms.o: $(SRCDIR)/asms.S
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

%.o: $(SRCDIR)/%.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

##Link
$(TARGET): $(OBJECTS)
#	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET) -Wl,-T linkerscript
	avr-ar rcs $(TARGET) $(OBJECTS)
	cp $(SRCDIR)/yaca-bl.h ../../include
	cp $(TARGET) ../lib

#%.lss: $(TARGET)
#	avr-objdump -h -S $< > $@
#??

#size: ${TARGET}
#	@echo
#	@avr-size ${TARGET}
#??

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) $(TARGET) *.d

mrproper: clean
	-rm -rf ../../include/yaca-bl.h ../lib/$(TARGET)

## Other dependencies
#-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

