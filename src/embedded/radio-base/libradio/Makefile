all:
	make -f Makefile.master
	make clean
	make -f Makefile.slave
	make clean

clean:
	rm -f *.o *.map *.elf *.hex *.s *.i

mrproper:
	rm -f *.a
