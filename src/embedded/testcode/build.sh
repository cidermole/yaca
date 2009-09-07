#!/bin/bash
/home/david/Code/svn-yaca/yaca-x86/yacac/trunk/build/main Switch
avr-gcc -L/usr/lib/yaca RSwitch.o Switch.o ftable.o -Wl,-T linkerscript -lyaca
avr-objcopy -O ihex -R .eeprom a.out a.hex
