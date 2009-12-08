avrdude -q -u -C /usr/local/etc/avrdude.conf -p m8 -c usbasp -U flash:w:MainPower.hex:a -U eeprom:w:MainPower.eep -v
