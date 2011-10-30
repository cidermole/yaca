#
# add all objects to build here...
#
OBJS = network.o
SRCDIR = ../../../src/x86/yaca-timed

#
# project library settings
#
EXE = main
#INCDIRS +=
#CFLAGS +=
LDFLAGS += -lrt

CROSS_COMPILE ?= /bulk/Build/yaca-toolchains/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-
CC = $(CROSS_COMPILE)g++

#########################################################################
#########################################################################

all: $(EXE)

clean:
	rm -f $(EXE)
	rm -f *.o

mrproper: clean
	rm -f ../../bin/yaca-timed

$(EXE):	$(OBJS)
	$(CC) -o $@ $(SRCDIR)/$(EXE).cpp $(OBJS) $(CFLAGS) $(LDFLAGS)
	cp $(EXE) ../../bin/yaca-timed

%.o: $(SRCDIR)/%.cpp
	$(CC) -c -o $@ $< $(INCDIRS) $(CFLAGS)
