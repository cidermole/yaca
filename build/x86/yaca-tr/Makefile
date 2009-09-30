#
# add all objects to build here...
#
OBJS =
SRCDIR = ../../../src/x86/yaca-tr

#
# project library settings
#
EXE = main
#INCDIRS +=
#CFLAGS +=

CC = gcc

#########################################################################
#########################################################################

all: $(EXE)

clean:
	rm -f $(EXE)
	rm -f *.o

mrproper: clean
	rm ../../bin/yaca-tr

$(EXE):	$(OBJS)
	$(CC) -o $@ $(SRCDIR)/$(EXE).c $(OBJS)
	cp $(EXE) ../../bin/yaca-tr

%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(INCDIRS) $(CFLAGS)
