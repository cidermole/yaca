#
# add all objects to build here...
#
OBJS = ihex.o config.o network.o
SRCDIR = ../../../src/x86/yaca-flash

#
# project library settings
#
EXE = main
#INCDIRS +=
CFLAGS = -fpermissive
LDFLAGS += -lrt

CC = g++

#########################################################################
#########################################################################

all: $(EXE)

clean:
	rm -f $(EXE)
	rm -f *.o

mrproper: clean
	rm -f ../../bin/yaca-flash

$(EXE):	$(OBJS)
	$(CC) -o $@ $(SRCDIR)/$(EXE).cpp $(OBJS) $(CFLAGS) $(LDFLAGS)
	cp $(EXE) ../../bin/yaca-flash

%.o: $(SRCDIR)/%.cpp
	$(CC) -c -o $@ $< $(INCDIRS) $(CFLAGS)
