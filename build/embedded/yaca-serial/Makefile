###############################################################################
# Makefile for the project yaca-serial
###############################################################################

## General Flags
PROJECT = yaca-serial
MCU = atmega8
TARGET = $(PROJECT).elf
CC = avr-gcc
SRCDIR = ../../../src/embedded/$(PROJECT)

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

F_CPU = 16000000UL
CLOCK_PRESCALER_MAGIC = MCP2515_CLKOUT_16MHZ

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -DF_CPU=$(F_CPU) -Os -fsigned-char -I/usr/lib/yaca/include
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
OBJECTS = $(PROJECT).o uart.o fifo.o calendar.o

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) $(PROJECT).hex $(PROJECT).eep $(PROJECT).lss size

## Compile

%.o: $(SRCDIR)/%.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

##Link
$(TARGET): $(OBJECTS)
	make -C ../libyaca-bl clean
	# rebuild libyaca-bl for our purposes (16 MHz)
	make -C ../libyaca-bl F_CPU=$(F_CPU) CLOCK_PRESCALER_MAGIC=$(CLOCK_PRESCALER_MAGIC)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) -o $(TARGET) $(LIBS)
	make -C ../libyaca-bl clean
	# rebuild libyaca-bl for others, like it was before
	make -C ../libyaca-bl

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) $(PROJECT).elf $(PROJECT).hex $(PROJECT).eep $(PROJECT).lss *.d




# deploy needs yaca-daemons (yaca-gw, yaca-ppp) stopped
# 'make deploy-wrt' for standalone use

before-deploy:
	ssh root@wrt 'mkdir -p /yaca/hex'
	scp $(PROJECT).hex root@wrt:/yaca/hex/yaca-serial.hex
	yaca-tr 3 0 00  # YacaSerial::reset()

deploy:
	ssh root@wrt '/yaca/bin/fboot /B9600 /D/dev/tts/1 /P/yaca/hex/yaca-serial.hex > /dev/null'

deploy-wrt:
	make before-deploy
	ssh yaca@nas '/home/yaca/yaca-daemons stop'
	ssh root@wrt 'killall yaca-ppp; killall yaca-gw'
	make deploy
	ssh root@wrt '/yaca/bin/yaca-gw /yaca/src/x86/yaca-gw/conf/yaca-gw.conf'
	ssh root@wrt '/yaca/bin/yaca-ppp'
	ssh yaca@nas '/home/yaca/yaca-daemons start'
