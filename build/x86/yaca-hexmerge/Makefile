#
# add all objects to build here...
#
OBJS = ihex.o config.o
SRCDIR = ../../../src/x86/yaca-hexmerge

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
	rm ../../bin/yaca-hexmerge

$(EXE):	$(OBJS)
	$(CC) -o $@ $(SRCDIR)/$(EXE).cpp $(OBJS)
	cp $(EXE) ../../bin/yaca-hexmerge

%.o: $(SRCDIR)/%.cpp
	$(CC) -c -o $@ $< $(INCDIRS) $(CFLAGS)
