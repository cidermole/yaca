#
# add all objects to build here...
#
OBJS = config.o network.o
SRCDIR = ../../../src/x86/yaca-logd

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
	rm -f ../../bin/yaca-logd

$(EXE):	$(OBJS)
	$(CC) -o $@ $(SRCDIR)/$(EXE).cpp $(OBJS) $(CFLAGS) $(LDFLAGS)
	cp $(EXE) ../../bin/yaca-logd

%.o: $(SRCDIR)/%.cpp
	$(CC) -c -o $@ $< $(INCDIRS) $(CFLAGS)
