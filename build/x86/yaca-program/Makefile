#
# add all objects to build here...
#
OBJS = ihex.o config.o network.o XmlTree.o
SRCDIR = ../../../src/x86/yaca-program

#
# project library settings
#
EXE = main
#INCDIRS +=
CFLAGS = -fpermissive

CC = g++

#########################################################################
#########################################################################

all: $(EXE)

clean:
	rm -f $(EXE)
	rm -f *.o

mrproper: clean
	rm -f ../../bin/yaca-program

$(EXE):	$(OBJS)
	$(CC) -o $@ $(SRCDIR)/$(EXE).cpp $(OBJS) $(CFLAGS)
	cp $(EXE) ../../bin/yaca-program

%.o: $(SRCDIR)/%.cpp
	$(CC) -c -o $@ $< $(INCDIRS) $(CFLAGS)
