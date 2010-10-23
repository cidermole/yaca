/************************************************************************/
/*                                                                      */
/*                     Bootloader Programmer                            */
/*      Linux-portation by Sascha Biedermann, sascha@coder-area.de      */
/*      Port version 1.4 by Andreas Butti, andreasbutti@bluewin.ch      */
/*         Originally written by Peter Dannegger, danni@specs.de        */
/*                                                                      */
/*                  TAB-Size : 3 Spaces                                 */
/************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "readargs.h"

//---------------------------------------------------------------------------

#define FALSE 	0
#define TRUE 	1

#define KEY_ENTER			0x0D
#define	ESCAPE				0xA5
#define COMMAND				0xA6
#define BADCOMMAND			0xA7	 	// command not supported
#define ANSWER				0xA8	 	// followed by length byte
#define CONTINUE			0xA9
#define SUCCESS				0xAA
#define FAIL				0xAB
#define	PROGEND				0xFF		 // A5, FF
#define	REVISION			0		// get bootloader revision
#define	BUFFSIZE			1		// get buffer size
#define	SIGNATURE			2		// get target signature
#define 	USERFLASH		3		// get user flash size
#define	PROGRAM				4		// program flash
#define	START				5		// start application
#define	CHECK_CRC			6		// CRC o.k.
#define	VERIFY				7		// Verify
#define	TIMEOUTSTART			1
#define	TIMEOUT				9		// 0.5s
#define	TIMEOUTP			180	 	// 10s
#define	MAXHEX				65535U 		// max flash size

//---------------------------------------------------------------------------------

void connect(void);
void get_crc(unsigned char d);
void helptext(void);
void getpasswd(void);
int  octal(char *p);
void program(char *fname, int verify);
int  read_crc(void);
int  readhex(FILE *fp, unsigned int *addr, unsigned char *data);
void read_info(void);
long readval(char type);
void sendbuff(unsigned int len, unsigned char *buff);
void sendcommand(unsigned char c);
int  check_crc(int flag);

void com_open(char device[], speed_t baud);
void com_close(void);
void com_putc(unsigned char c);
int  com_getc(int timeout);
void com_puts(char *text);
void _com_puts(char *text);

//--------------------------------------------------------------------------

#undef  DEBUG
#define DEBUG

#define BAUD_DEF 	16		 //Default baudrate

#define DEVICE "/dev/ttyS0"	 	//Default Device

char device[]= DEVICE;

speed_t baud_const[] = { B50, B75,   B110,  B134,  B150,  B200,   B300,   B600,   B1200, 
			      B1800, B2400, B4800, B9600, B19200, B38400, B57600, B115200
                        };

unsigned long baud_value[] = { 50,   75,  110,  134,  150,   200,   300,   600, 1200, 
				   1800, 2400, 4800, 9600, 19200, 38400, 57600, 115200
                             };

char 		Passwd[130] = "Peda";
char 		Flash[130] = "";

unsigned int 	crc;
int 		baudrate= BAUD_DEF;
long 		flashsize;
int 		buffersize;

struct termios 	oldtio;
int 		fd;			 	//Serielle Schnitstelle (Stream)
int 		_argc;
char 		**_argv;

//-------------------------------------------------------------------------------------

void helptext(void)
 {
     printf("\n\t /?            \t\t Get this help message\n"
		"\t /Bnnnn        \t\t Define baud rate\n"
		"\t /Ddevice      \t\t Define serial port\n"
		"\t /PFname,Ename \t\t Perform Program\n"
		"\t /VFname,Ename \t\t Perform Verify\n"
		"\n\n"
		// "\t /D         \t\t Debug Mode\n\n"
           );
     exit(1);
}

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------

int main( int argc, char *argv[])
 {
	clock_t 	t;		  	// Zeitmessung
	unsigned long 	tmp_baud;	  	// Baudrate einlesen
	char    	s[32];
	int     	i;

	_argc = argc;
	_argv = argv;

	if( argc < 2 || readargs( ABOOL, '?', &i) ) 
		helptext();

	getpasswd();


	if( readargs( ALONG, 'b', &tmp_baud) )    // Prüfen, ob die Baudrate gültig ist
          {
	    baudrate = -1;
            
  	    i=0; while( i< sizeof(baud_value)/sizeof(baud_value[0])) 
                 {
	  	   if( baud_value[i++] == tmp_baud) {
				baudrate = i;
				break;
	   	   	   }
		}
	    if( baudrate < 0 ) { printf("Ungültige Baudrate!"); helptext(); }
	   }


	if (readargs( ASTRING, 'd', &s) )  // Device einlesen
		strcpy(device, s);
	

	// Sooo... jetzt habe wir alles, auf zum Programmieren...

	com_open(device, baud_const[baudrate]);

	if (fd < 0) { perror(device); exit(-1); }


	connect();		 // connect ATMega
	check_crc( 0);
	read_info();

	t= clock(); 		//Starttime

	if( readargs( ASTRING, 'p', &Flash) ) 		 // Programming
	    if( Flash[0] ) program(Flash, 0);
	

	if( readargs( ASTRING, 'v', &Flash) ) 		 // Verify
	    if( Flash[0] ) program(Flash, 1);
	

	check_crc(1);

	sendcommand( START );

	printf("Elapsed time: %4.2f seconds\n",(float)(clock()-t)/CLOCKS_PER_SEC);

	com_close();

	return 0;
}

//-------------------------------------------------------------------------------------

int check_crc(int flag) {

	int i;
	unsigned int crc1;

	sendcommand( CHECK_CRC);
	crc1 = crc;
	com_putc(crc1 );
	com_putc(crc1 >> 8);

	i = com_getc( TIMEOUT);

	switch (i ) {
		case SUCCESS:
			if (flag )
				printf("CRC: o.k.\n");
			return 0;

		case BADCOMMAND:
			if (flag )
				printf("CRC: not implemented\n");
			return 0;

		case FAIL:
			if (flag )
				printf("CRC: error !\n");

		default:
			return 1;
	}
}

//-------------------------------------------------------------------------------------

void program(char *fname, int verify) {
	FILE *fp;
	unsigned int lastaddr = 0;
	unsigned int addr;
	int i;
	unsigned char *data;
	unsigned char s[255];
	unsigned char d1;
	clock_t t = clock();

	data = malloc( MAXHEX);
	if (data == NULL) {
		puts("Memory allocation error !");
		return;
	}
	memset(data, 0xFF, MAXHEX);
	if ( NULL == ( fp = fopen( fname, "rb" ) )) {
		printf("File %s open failed !\n", fname);
		return;
	}

	printf("reading file... ");

	while ( (i = readhex(fp, &addr, s )) >= 0) {
		if (i ) {
			memcpy(data + addr, s, i );
			addr += i;
			if (lastaddr < addr-1)
				lastaddr = addr-1;
			addr++;
		}
	}

	printf("Done.\n");

	fclose(fp );
	if (verify == 0) {
		printf("Program %s: 0000 - 0000", fname);
		sendcommand( PROGRAM);
	} else {
		tcflush(fd, TCOFLUSH);
		if (com_getc( TIMEOUT) == BADCOMMAND) {
			printf("Verify not implemented !\n");
			free(data );
			return;
		}
		printf("Verify %s: 0000 - 0000", fname);
	}

	fflush(stdout);

	for (i = buffersize, addr = 0;; addr++) {
		d1 = data[addr];
		//in den Buffer schreiben
		write(fd, &d1, 1);
		get_crc(d1);

		if (d1 == ESCAPE)
			com_putc( ESCAPE); // A5,A5 = A5

		if ( --i == 0) {
			//warten bis gesendet
			tcdrain(fd);
			if (clock() - t > 9) {
				t += 9;
				printf("\b\b\b\b%04X", addr + 1);
				fflush(stdout);
			}
			if ( !verify && (i = com_getc( TIMEOUTP)) != CONTINUE) {
				printf(" failed !");
				fflush(stdout);
				break;
			}
			i = buffersize;
		}
		if (addr == lastaddr ) {
			com_putc( ESCAPE);
			com_putc( PROGEND); // A5,FF = End
			printf("\b\b\b\b%04X", addr );
			fflush(stdout);
			if (com_getc( TIMEOUTP) == SUCCESS) {
				printf(" successful");
			} else {
				printf(" failed !");
			}
			break;
		}
	}
	free(data );
	printf("\n");
}

//-------------------------------------------------------------------------------------

int sscanhex(unsigned char *str, unsigned int *hexout, int n) {
	unsigned int hex = 0, x = 0;
	for (; n; n--) {
		x = *str;
		if (x >= 'a')
			x += 10 - 'a';
		else if (x >= 'A')
			x += 10 - 'A';
		else
			x -= '0';
		if (x >= 16)
			break;
		hex = hex * 16+ x;
		str++;
	}
	*hexout = hex;
	return n; // 0 if all digits read
}

//-------------------------------------------------------------------------------------

int readhex(FILE *fp, unsigned int *addr, unsigned char *data) {
	/* Return value: 1..255	number of bytes
	 0	end or segment record
	 -1	file end
	 -2	error or no HEX-File */
	char hexline[524]; // intel hex: max 255 byte
	char * hp = hexline;
	unsigned int byte;
	int i;
	unsigned int num;

	if (fgets(hexline, 524, fp ) == NULL)
		return -1; // end of file
	if ( *hp++ != ':')
		return -2; // no hex record
	if (sscanhex(hp, &num, 2))
		return -2; // no hex number
	hp += 2;
	if (sscanhex(hp, addr, 4))
		return -2;
	hp += 4;
	if (sscanhex(hp, &byte, 2))
		return -2;
	if (byte != 0) // end or segment record
		return 0;
	for (i = num; i--;) {
		hp += 2;
		if (sscanhex(hp, &byte, 2))
			return -2;
		*data++ = byte;
	}
	return num;
}

//-------------------------------------------------------------------------------------

long readval(char type)
 {
     unsigned int  j  = 257;
              int  i;
     unsigned long val= 0;

#ifdef  DEBUG
  		   printf(" readval: \n");
#endif

     while (1) 
        {
 	   if( (i= com_getc( TIMEOUT)) == -1 ) return -1;

#ifdef  DEBUG
           printf(" %02x val %8ld \t j %3u \n", (unsigned char)i,val,j); fflush(stdout);
#endif

	   switch ( j )
            {
		case   2:
		case   3:
		case   4: j--; val = val * 256+ i;
		 	  break;

		case 256: j = i; // if (type) j--;
			  break;

		case 257: if (i == FAIL  ) return -2;
			  if (i == ANSWER) j = 256;
			  break;

		case 1 :  if (i == SUCCESS ) 
				return val;

		default:  	return -2;
	     }
	}
 }

//-------------------------------------------------------------------------------------

void read_info(void) {
	unsigned long i;
	char *s = "";

	sendcommand( REVISION);
	i = readval(0);
	printf("Bootloader V%lX.%lX\n", i>>8, i&0xFF);

	sendcommand( SIGNATURE);
	i = readval(0);
	switch (i ) {
	case 0x1e9007:
		s = "ATtiny13";
		break;
	case 0x1e910A:
		s = "ATtiny2313";
		break;
	case 0x1e9206:
		s = "ATtiny45";
		break;
	case 0x1e9205:
		s = "ATmega48";
		break;
	case 0x1e9307:
		s = "ATmega8";
		break;
	case 0x1e9403:
		s = "ATmega16";
		break;
	case 0x1e9406:
		s = "ATmega168";
		break;
	case 0x1e9502:
		s = "ATmega32";
		break;
	case 0x1e9609:
		s = "ATmega644";
		break;
	}
	printf("Target: %06lX %s\n", i, s );

	sendcommand( BUFFSIZE );
	i = readval(1);
	buffersize = i;
	printf("Buffer: %ld Byte\n", i );

	sendcommand( USERFLASH );
	i = readval(1);
	flashsize = i;
	printf("Size available: %ld Byte\n", i );
}

//-------------------------------------------------------------------------------------

int octal(char *p) { // read octals "\123"
	int n, i;

	if ( *p++ != '\\')
		return -1; // no octal number
	for (n = 0, i = 3; i; i--) {
		if ( *p == 0)
			return -2; // wrong octal number
		n = n * 8+ *p++- '0';
	}
	return n;
}

//-------------------------------------------------------------------------------------

void getpasswd(void) {
	char text[81], * t = text, * p = Passwd;
	int i;

	if (readargs( ASTRING, 'i', t) && *t ) {
		while ( *t ) { // copy string
			i = octal(t ); // convert octals
			if (i >= 0) {
				*p = i;
				t += 3; // octal = 3 bytes
			} else {
				*p = *t;
			}
			p++;
			t++;
		}
		*t = 0;
	}
}

//-------------------------------------------------------------------------------------

void connect(void) {
	char WAITSTRING[] = { '|', '/', '-', '\\' };
	unsigned int i = 1;
	printf("\n");
	while (1) {
		printf("\033[A%s at %ld Baud: %c\n", device, baud_value[baudrate],
				WAITSTRING[++i&3]);
		_com_puts(Passwd);
		if (com_getc( 0) == SUCCESS) {
			printf("Connected\n");
			com_getc ( 2);
			tcflush(fd, TCIOFLUSH);
			return;
		}
	}
}

//-------------------------------------------------------------------------------------

void get_crc(unsigned char d) {
	int i;

	crc ^= d;
	for (i = 8; i; i--) {
		crc = (crc >> 1) ^ ((crc & 1) ? 0xA001 : 0 );
	}
}

//-------------------------------------------------------------------------------------

void sendbuff(unsigned int len, unsigned char *buff) {
	while (len--)
		com_putc( *buff++);
}

//-------------------------------------------------------------------------------------

void sendcommand(unsigned char c) {
	com_putc( COMMAND);
	com_putc(c );
}

//-------------------------------------------------------------------------------------
/**
 Kommunkation
 */

void com_open(char device[], speed_t baud) {
	struct termios newtio;

	// Erstmal das Device �ffnen
	fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0) {
		return;
	}

	// Jetzt die Einstellungen Speichern
	tcgetattr(fd, &oldtio);

	// Den Neuen Struct mit 0 initzialisieren
	//memset(&newtio, 0x00 , sizeof(newtio));

	// Flags setzen
	newtio.c_cflag = CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR | IGNBRK;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;

	//cfmakeraw(&newtio);

	// Timeout in 100ms
	newtio.c_cc[VTIME] = 0;
	// 1 Zeichen lesen
	newtio.c_cc[VMIN] = 0;

	// Baudrate einstellen
	cfsetispeed(&newtio, baud);
	cfsetospeed(&newtio, baud);

	// Sende- und com_getcspuffer leeren
	tcflush(fd, TCIOFLUSH);

	// die neuen Port-Einstellungen setzen
	tcsetattr(fd, TCSANOW, &newtio);

	return;
}

//-------------------------------------------------------------------------------------

void com_close(void) {
	// die alten Einstellung wiederherstellen
	tcsetattr(fd, TCSANOW, &oldtio);

	// und das Device schliessen
	close(fd);
}

//-------------------------------------------------------------------------------------

void com_putc(unsigned char c) {
	tcdrain(fd);
	write(fd, &c, 1);
	get_crc(c );
}

//-------------------------------------------------------------------------------------

int com_getc(int timeout) 
 {
	unsigned char c= 0;
	clock_t       t= clock();

	do { if( read(fd, &c, 1) == 1 ) return c;
	
	   } while( (clock()-t)/CLOCKS_PER_SEC < timeout );

	return -1;
 }

//-------------------------------------------------------------------------------------

void _com_puts(char *text) {
	while ( *text )
		com_putc( *text++);
}

void com_puts(char *text) {
	_com_puts(text);
	com_putc( KEY_ENTER);
}

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
