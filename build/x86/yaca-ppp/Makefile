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

CC = g++

#########################################################################
#########################################################################

all: $(EXE)

clean:
	rm -f $(EXE)
	rm -f *.o

mrproper: clean
	rm ../../bin/yaca-ppp

$(EXE):	$(OBJS)
	$(CC) -o $@ $(SRCDIR)/$(EXE).cpp $(OBJS)
	cp $(EXE) ../../bin/yaca-ppp

%.o: $(SRCDIR)/%.cpp
	$(CC) -c -o $@ $< $(INCDIRS) $(CFLAGS)
