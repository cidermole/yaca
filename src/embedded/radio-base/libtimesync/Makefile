#
#
# F_CPU: Defines CPU frequency e.g. for <util/delay.h>
#
F_CPU = 2000000UL

#
# Target
#
TARGET = libtimesync.a

#
# MCU
#
MCU = atmega8

OBJECTS = timesync.o

CC = avr-gcc
OBJCOPY = avr-objcopy
COMMON = -mmcu=$(MCU)
CFLAGS = $(COMMON) -Wall -gdwarf-2 -DF_CPU=$(F_CPU) -Os -fsigned-char
LDFLAGS = $(COMMON)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	avr-ar rcs $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o *.map *.elf *.hex *.s *.i
