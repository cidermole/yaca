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

CC = /home/david/openwrt/trunk/staging_dir/toolchain-mipsel_gcc3.4.6/bin/mipsel-linux-g++

#########################################################################
#########################################################################

all: $(EXE)

clean:
	rm -f $(EXE)
	rm -f *.o

mrproper: clean

$(EXE):	$(OBJS)
	$(CC) -o $@ $(SRCDIR)/$(EXE).cpp $(OBJS) $(LDFLAGS)

%.o: $(SRCDIR)/%.cpp
	$(CC) -c -o $@ $< $(INCDIRS) $(CFLAGS)
