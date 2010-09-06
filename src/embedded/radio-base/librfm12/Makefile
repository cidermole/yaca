#
#
# F_CPU: Defines CPU frequency e.g. for <util/delay.h>
#
F_CPU = 16000000UL

#
# Target
#
TARGET = funk-empfang

#
# MCU
#
MCU = atmega8

OBJECTS = rfm12_phy.o rfm12_mac.o rfm12_llc.o hamming.o
# lcd-routines.o owdebug.o

CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS = -mmcu=$(MCU) -Wall -gdwarf-2 -DF_CPU=$(F_CPU) -Os -fsigned-char -save-temps

all: $(TARGET).hex

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -j .data -j .text -O ihex $< $@

$(TARGET).elf: $(TARGET).o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ -Wl,-Map,$(TARGET).map $< $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

load: $(TARGET).hex
	avrdude -q -u -C /usr/local/etc/avrdude.conf -p m8 -c usbasp -U flash:w:$(TARGET).hex:a -v

clean:
	rm -f *.o *.map *.elf *.hex *.s *.i
