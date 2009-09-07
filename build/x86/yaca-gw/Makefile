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

CC = /home/david/openwrt/trunk/staging_dir/toolchain-mipsel_gcc3.4.6/bin/mipsel-linux-gcc

#########################################################################
#########################################################################

all: $(EXE)

clean:
	rm -f $(EXE)
	rm -f *.o

$(EXE):	$(OBJS)
	$(CC) -o $@ $(SRCDIR)/$(EXE).c $(OBJS)

%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(INCDIRS) $(CFLAGS)
