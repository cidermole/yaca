#!/bin/bash
../../../build/bin/yaca-c Switch
avr-gcc -L../../../build/lib RSwitch.o Switch.o ftable.o -Wl,-T linkerscript -lyaca
avr-objcopy -O ihex -R .eeprom a.out a.hex
