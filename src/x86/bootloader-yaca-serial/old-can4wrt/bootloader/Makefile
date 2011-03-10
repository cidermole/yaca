TRG = fboot

SRC = $(TRG).c readargs.c
OBJ = $(SRC:.c=.o)

CCFLAGS = -Wall -g

all : $(TRG)

%.o : %.c
	/home/david/openwrt/trunk/staging_dir/toolchain-mipsel_gcc3.4.6/bin/mipsel-linux-gcc $(CCFLAGS) -c $< -o $@

$(TRG) : $(OBJ)    
	/home/david/openwrt/trunk/staging_dir/toolchain-mipsel_gcc3.4.6/bin/mipsel-linux-gcc $(CCFLAGS) $(OBJ) -o $@

clean:
	rm -f $(OBJ)
	rm -f $(TRG)
