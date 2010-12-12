#
#
# F_CPU: Defines CPU frequency e.g. for <util/delay.h>
#
F_CPU = 1000000UL

#
# Target
#
TARGET = main

#
# MCU
#
MCU = atmega8

OBJECTS = one-wire.o synctime.o


CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS = -mmcu=$(MCU) -Wall -gdwarf-2 -DF_CPU=$(F_CPU) -Os -fsigned-char -save-temps
# use yaca linker script to get .tables section
LDFLAGS = -L../libradio -lradio-slave -Wl,-T ../../../x86/yaca-c/link-atmega8.txt

all: $(TARGET).hex

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -j .data -j .text -O ihex $< $@

$(TARGET).elf: $(TARGET).o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ -Wl,-Map,$(TARGET).map $< $(OBJECTS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

load: $(TARGET).hex
	avrdude -q -u -C /usr/local/etc/avrdude.conf -p m8 -c usbasp -U flash:w:$(TARGET).hex:a -v

clean:
	rm -f *.o *.map *.elf *.hex *.s *.i
