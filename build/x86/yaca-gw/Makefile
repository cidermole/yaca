#
# add all objects to build here...
#
OBJS = config.o socket.o serial.o hostlist.o
SRCDIR = ../../../src/x86/yaca-gw

#
# project library settings
#
EXE = main
#INCDIRS +=
#CFLAGS +=

CROSS_COMPILE ?= /bulk/yaca-toolchains/staging_dir/toolchain-mipsel_gcc3.4.6/bin/mipsel-linux-
CC = $(CROSS_COMPILE)gcc

#########################################################################
#########################################################################

all: $(EXE)

clean:
	rm -f $(EXE)
	rm -f *.o

mrproper: clean
	rm -f ../../bin/yaca-gw

deploy:
	ssh root@wrt 'mkdir -p /yaca/bin; mkdir -p /yaca/src/x86/yaca-gw/conf'
	scp $(EXE) root@wrt:/yaca/bin/yaca-gw
	scp ../../../src/x86/yaca-gw/conf/yaca-gw.conf root@wrt:/yaca/src/x86/yaca-gw/conf/

$(EXE):	$(OBJS)
	$(CC) -o $@ $(SRCDIR)/$(EXE).c $(OBJS)
	cp $(EXE) ../../bin/yaca-gw

%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(INCDIRS) $(CFLAGS)
