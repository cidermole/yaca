all: aembedded ax86
	echo $(flags)
# 'make flags=-DYACA_INT1'

clean:
	cd embedded; make clean
	cd x86; make clean

mrproper:
	cd embedded; make mrproper
	cd x86; make mrproper

aembedded:
	cd embedded; make

ax86:
	cd x86; make
