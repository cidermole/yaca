#!/bin/bash

echo Enter the name of the node that caused the mayhem, then connect programmer and press return:
read node

today=`date "+%Y-%m-%d_%H-%M-%S"`
dir="${node}_${today}"

mkdir $dir
cd $dir

avrdude -u -C /usr/local/etc/avrdude.conf -p m8 -c usbasp -U flash:r:flash.hex:i -v
avrdude -u -C /usr/local/etc/avrdude.conf -p m8 -c usbasp -U eeprom:r:eeprom.eep:i -v
avrdude -u -C /usr/local/etc/avrdude.conf -p m8 -c usbasp -U hfuse:r:hfuse.hex:i -v
avrdude -u -C /usr/local/etc/avrdude.conf -p m8 -c usbasp -U lfuse:r:lfuse.hex:i -v

