-> combine Switch.xml (auto-generated) and Switch1.xml (could have another name, given on command line) to an eeprom source file uint8_t EEMEM asdf[512] = { ... }; , compile and objcopy to obtain an eeprom hex. program that into node id given.

avr-gcc -mmcu=atmega8 -o test.o test.c -c
avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex test.o main.eep


Bootstrap of remote call headers (RSwitch.h)
- Include a Messages.h with
	#define SendMessage(a, b)
- If remote call header file (RSwitch.h) doesn't exist yet, create...
- Compile the source (Switch.C)
- Generate the remote call headers (RSwitch.h)
- Include a normal Messages.h
- Compile again


avr-gcc -L/usr/lib/yaca RSwitch.o Switch.o ftable.o -Wl,-T linkerscript -lyaca -lyaca-client
avr-objcopy -O ihex -R .eeprom a.out a.hex

/home/david/Code/svn-yaca/yaca-x86/ycflash/trunk/build/main a.hex
/home/david/Code/svn-yaca/yaca-x86/ycflash/branches/yctr/build/main 1 0 0E
