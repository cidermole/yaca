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

CC = g++

#########################################################################
#########################################################################

all: $(EXE)

clean:
	rm -f $(EXE)
	rm -f *.o

mrproper: clean
	rm ../../bin/yaca-timed

$(EXE):	$(OBJS)
	$(CC) -o $@ $(SRCDIR)/$(EXE).cpp $(OBJS) $(CFLAGS) $(LDFLAGS)
	cp $(EXE) ../../bin/yaca-timed

%.o: $(SRCDIR)/%.cpp
	$(CC) -c -o $@ $< $(INCDIRS) $(CFLAGS)
