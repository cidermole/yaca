#
#
# F_CPU: Defines CPU frequency e.g. for <util/delay.h>
#
F_CPU = 2000000UL

#
# Target
#
TARGET = libradio.a

#
# MCU
#
MCU = atmega8

#OBJECTS = radio.o rijndael.o
OBJECTS = radio.o aestable.o slave.o

#CC = avr-gcc
CC = gcc
#ARCHIVER = avr-ar
ARCHIVER = ar
#COMMON = -mmcu=$(MCU)
COMMON = -g
#CFLAGS = $(COMMON) -Wall -gdwarf-2 -DF_CPU=$(F_CPU) -Os -fsigned-char
CFLAGS = $(COMMON) -Wall
LDFLAGS = $(COMMON)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(ARCHIVER) rcs $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

rijndael.o: rijndael.S
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o *.map *.elf *.hex *.s *.i

test:
	#$(CC) $(CFLAGS) test.c -o ptest -Wl,-T linkerscript.txt -L. -lradio
	$(CC) $(CFLAGS) test.c -o ptest -L. -lradio
