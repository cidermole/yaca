#
# add all objects to build here...
#
OBJS = readargs.o
SRCDIR = ../../../src/x86/bootloader-yaca-serial

#
# project library settings
#
EXE = fboot
#INCDIRS +=
CFLAGS += -Wall

CROSS_COMPILE ?= /bulk/Build/yaca-toolchains/staging_dir/toolchain-mipsel_gcc3.4.6/bin/mipsel-linux-
CC = $(CROSS_COMPILE)gcc

#########################################################################
#########################################################################

all: $(EXE)

clean:
	rm -f $(EXE)
	rm -f *.o

mrproper: clean
	rm -f ../../bin/fboot

deploy:
	ssh root@wrt 'mkdir -p /yaca/bin'
	scp $(EXE) root@wrt:/yaca/bin/fboot

$(EXE):	$(OBJS)
	$(CC) -o $@ $(SRCDIR)/$(EXE).c $(OBJS)
	cp $(EXE) ../../bin/fboot

%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(INCDIRS) $(CFLAGS)
