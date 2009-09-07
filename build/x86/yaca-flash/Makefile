#
# add all objects to build here...
#
OBJS = ihex.o
SRCDIR = ../../../src/x86/yaca-flash

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

$(EXE):	$(OBJS)
	$(CC) -o $@ $(SRCDIR)/$(EXE).c $(OBJS)
	cp $(EXE) ../../bin/yaca-flash

%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(INCDIRS) $(CFLAGS)
