#
# add all objects to build here...
#
OBJS = config.o network.o
SRCDIR = ../../../src/x86/yaca-cached

#
# project library settings
#
EXE = main
#INCDIRS +=
CFLAGS += -g
LDFLAGS += -lrt

CROSS_COMPILE ?= /bulk/yaca-toolchains/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-
CC = $(CROSS_COMPILE)g++

#########################################################################
#########################################################################

all: $(EXE)

clean:
	rm -f $(EXE)
	rm -f *.o

mrproper: clean
	rm ../../bin/yaca-cached

$(EXE):	$(OBJS)
	$(CC) -o $@ $(SRCDIR)/$(EXE).cpp $(OBJS) $(CFLAGS) $(LDFLAGS)
	cp $(EXE) ../../bin/yaca-cached

%.o: $(SRCDIR)/%.cpp
	$(CC) -c -o $@ $< $(INCDIRS) $(CFLAGS)
