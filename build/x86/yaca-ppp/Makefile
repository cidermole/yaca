#
# add all objects to build here...
#
OBJS = config.o network.o
SRCDIR = ../../../src/x86/yaca-ppp

#
# project library settings
#
EXE = main
#INCDIRS +=
#CFLAGS +=
LDFLAGS = -lutil

CROSS_COMPILE ?= /bulk/yaca-toolchains/staging_dir/toolchain-mipsel_gcc3.4.6/bin/mipsel-linux-
CC = $(CROSS_COMPILE)gcc

#########################################################################
#########################################################################

all: $(EXE)

clean:
	rm -f $(EXE)
	rm -f *.o

mrproper: clean
	rm -f ../../bin/yaca-ppp

deploy:
	ssh root@wrt 'mkdir -p /yaca/bin; mkdir -p /yaca/src/x86/yaca-ppp/conf'
	scp $(EXE) root@wrt:/yaca/bin/yaca-ppp
	scp ../../../src/x86/yaca-ppp/conf/yaca-ppp.conf root@wrt:/yaca/src/x86/yaca-ppp/conf/
	scp ../../../src/x86/yaca-ppp/conf/yaca-chat-script root@wrt:/etc/ppp/yaca-chat-script

$(EXE):	$(OBJS)
	$(CC) -o $@ $(SRCDIR)/$(EXE).c $(OBJS) $(LDFLAGS)
	cp $(EXE) ../../bin/yaca-ppp

%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(INCDIRS) $(CFLAGS)
