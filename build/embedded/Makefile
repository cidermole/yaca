all: abootloader ayaca-serial alibyaca alibyaca-bl

clean:
	cd bootloader; make clean
	cd yaca-serial; make clean
	cd libyaca; make clean
	cd libyaca-bl; make clean

mrproper:
	cd bootloader; make clean
	cd yaca-serial; make clean
	cd libyaca; make mrproper
	cd libyaca-bl; make mrproper

abootloader: alibyaca-bl
	cd bootloader; make

ayaca-serial: alibyaca alibyaca-bl
	cd yaca-serial; make

alibyaca:
	cd libyaca; make

alibyaca-bl:
	cd libyaca-bl; make
