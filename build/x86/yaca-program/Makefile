#
# add all objects to build here...
#
OBJS = ihex.o config.o network.o
SRCDIR = ../../../src/x86/yaca-program

#
# project library settings
#
EXE = main
#INCDIRS +=
#CFLAGS +=

CC = g++

#########################################################################
#########################################################################

all: $(EXE)

clean:
	rm -f $(EXE)
	rm -f *.o

mrproper: clean
	rm ../../bin/yaca-program

$(EXE):	$(OBJS)
	$(CC) -o $@ $(SRCDIR)/$(EXE).cpp $(OBJS)
	cp $(EXE) ../../bin/yaca-program

%.o: $(SRCDIR)/%.cpp
	$(CC) -c -o $@ $< $(INCDIRS) $(CFLAGS)
