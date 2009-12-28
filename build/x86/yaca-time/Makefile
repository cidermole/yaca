#
# add all objects to build here...
#
OBJS = network.o
SRCDIR = ../../../src/x86/yaca-time

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
	rm ../../bin/yaca-time

$(EXE):	$(OBJS)
	$(CC) -o $@ $(SRCDIR)/$(EXE).cpp $(OBJS)
	cp $(EXE) ../../bin/yaca-time

%.o: $(SRCDIR)/%.cpp
	$(CC) -c -o $@ $< $(INCDIRS) $(CFLAGS)
