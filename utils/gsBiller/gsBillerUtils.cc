#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <array>

#include <cunistd>

#include "gsBillerUtils.h"

using namespace std;

#include "protocol.h"
#include "rs232.c"

#define COM_TIMEOUT     -1
#define COM_DISCONNECT  -2

// Definitions
#define TIMEOUT   3   // 0.3s
#define TIMEOUTP  40  // 4s

// enum for autoreset
typedef enum {
    NO_AUTORESET  = 0,      // don't reset
    AUTORESET
} autoreset_t;

/**************************************************************/
/*                          GLOBALS                           */
/**************************************************************/
static int              running = TRUE;
static int              esc_seq = 0;

// default is Reset via DTR
static autoreset_t autoreset = AUTORESET;
static int              bsize = 16;

// pointer to password...
// following characters are needed for autobaud
// 0x0A - LF,  0x0B - VT,  0x0D - CR,  0x0F - SI
// 0x21 - '!', 0x43 - 'C', 0x61 - 'a', 0x85, 0x87
// 0xC3 - 'A~',0xE1 - 'a´' - ISO8859-1
static char             *password = "GurU";
static unsigned int     device = 0;
static int              baud = 9600;

// Filename of the HEX File
static const char * hexfile = "prog.hex";

typedef struct bootInfo
{
    long    revision;
    long    signature;
    long    buffsize;
    long    flashsize;
    int     crc_on;
    int     blocksize;
} bootInfo_t;

typedef struct
{
  unsigned long   id;
  const char      *name;
} avrdev_t;
avrdev_t avr_dev[] = {
  { 0x01e9502, "ATmega32" },
};

// CRC checksum
unsigned int crc = 0;

/// Prototypes
void calc_crc(unsigned char d);

/**
 * reads hex data from string
 */
int sscanhex (char          *str,
              unsigned int  *hexout,
              int           n)
{
    unsigned int hex = 0, x = 0;

    for(; n; n--)
    {
        x = *str;
        if(x >= 'a')
        {
            x += 10 - 'a';
        }
        else if(x >= 'A')
        {
            x += 10 - 'A';
        }
        else
        {
            x -= '0';
        }

        if(x >= 16)
        {
            break;
        }

        hex = hex * 16 + x;
        str++;
    }

    *hexout = hex;
    return n; // 0 if all digits
}


/**
 * Reads the hex file
 *
 * @return 1 to 255 number of bytes, -1 file end, -2 error or no HEX File
 */
int readhex (FILE           *fp,
             unsigned long  *addr,
             unsigned char  *data)
{
    char hexline[524]; // intel hex: max 255 byte
    char *hp = hexline;
    unsigned int byte;
    int i;
    unsigned int num;
    unsigned int low_addr;

    if(fgets( hexline, 524, fp ) == NULL)
    {
        return -1; // end of file
    }

    if(*hp++ != ':')
    {
        return -2; // no hex record
    }

    if(sscanhex(hp, &num, 2))
    {
        return -2; // no hex number
    }

    hp += 2;

    if(sscanhex(hp, &low_addr, 4))
    {
        return -2;
    }

    *addr &= 0xF0000L;
    *addr += low_addr;
    hp += 4;

    if(sscanhex( hp, &byte, 2))
    {
        return -2;
    }

    if(byte == 2)
    {
        hp += 2;
        if(sscanhex(hp, &low_addr, 4))
        {
            return -2;
        }
        *addr = low_addr * 16L;
        return 0; // segment record
    }

    if(byte == 1)
    {
        return 0; // end record
    }

    if(byte != 0)
    {
        return -2; // error, unknown record
    }

    for(i = num; i--;)
    {
        hp += 2;
        if(sscanhex(hp, &byte, 2))
        {
            return -2;
        }
        *data++ = byte;
    }
    return num;
}

/**
 * Read a hexfile
 */
char * read_hexfile(const char * filename, unsigned long * lastaddr)
{
    char    *data;
    FILE    *fp;
    int     len;
    int     x;
    unsigned char line[256];
    unsigned long addr = 0;

    data = malloc(MAXFLASH);
    if (data == NULL)
    {
        printf("Memory allocation error, could not get %d bytes for flash-buffer!\n",
               MAXFLASH);
        return NULL;
    }

    *lastaddr = 0;
    memset (data, 0xff, MAXFLASH);

    if(NULL == (fp = fopen(filename, "r")))
    {
        printf("File \"%s\" open failed: %s!\n\n", filename, strerror(errno));
        free(data);
        return NULL;
    }

    printf("Reading       : %s... ", filename);


    // reading file to "data"
    while((len = readhex(fp, &addr, line)) >= 0)
    {
        if(len)
        {
            if( addr + len > MAXFLASH )
            {
                fclose(fp);
                free(data);
                printf("\n  Hex-file too large for target!\n");
                return NULL;
            }
            for(x = 0; x < len; x++)
            {
                data[x + addr] = line[x];
            }

            addr += len;

            if(*lastaddr < (addr-1))
            {
                *lastaddr = addr-1;
            }
            addr++;
        }
    }

    fclose(fp);

    printf("File read.\n");
    return data;
}


/**
 * Calculate the new CRC sum
 */
void calc_crc(unsigned char d)
{
    int i;

    crc ^= d;
    for( i = 8; i; i-- )
    {
        crc = (crc >> 1) ^ ((crc & 1) ? 0xA001 : 0 );
    }
}


void com_putc(int device, unsigned char c)
{
  RS232_SendByte(device, c);
  calc_crc( c ); // calculate transmit CRC
  RS232_flushTX(device);
}

/**
 * Sending a command
 */
void sendcommand(int device, unsigned char c)
{
    RS232_SendByte(device, COMMAND);
    calc_crc( COMMAND ); // calculate transmit CRC
    RS232_SendByte(device, c);
    calc_crc( c ); // calculate transmit CRC
    RS232_flushTX(device);
}


static unsigned int baudrates[] = {
  50,
  75,
  110,
  134,
  150,
  200,
  300,
  600,
  1200,
  1800,
  2400,
  4800,
  9600,
  19200,
  38400,
  57600,
  115200,
  230400
};

/**
 * Get status of device. It might happen that the device disappears (e.g.
 * due to a USB-serial unplug).
 *
 * @return 1 if device seems ok, 0 if it seems not available
 */
int get_device_status(int device)
{
  int status;

  if (0 == GetCommModemStatus(Cport[device], (LPDWORD)((void *)&status)))
    return 1;

  return 0;
}


#define RBUF_SZ  1024
char readbuf[RBUF_SZ];
unsigned int rbuf_valid = 0;

/**
 * Receives one char or -1 if timeout
 * timeout in 10th of seconds
 */
char com_getc(int device,
             int timeout)
{
    static long         ticks = 0;
    char    c;

#define TICK_MAX 10000

  for (ticks = 0; (rbuf_valid <= 0) && (ticks < TICK_MAX); ticks++)
  {
    rbuf_valid = RS232_PollComport(device, readbuf, RBUF_SZ);
    if (rbuf_valid > 0) {
      break;
    } else
      rbuf_valid = 0;
  }
  if (ticks == TICK_MAX)
    return COM_TIMEOUT;

  rbuf_valid--;
  c = readbuf[0];
  memcpy(readbuf, readbuf+1, rbuf_valid);
  return c;
}

/**
 * Reads a value from bootloader
 *
 * @return value; -2 on error; exit on timeout
 */
long readval(int fd)
{
    int i;
    int j = 257;
    long val = 0;

    while(1)
    {
        i = com_getc (fd, TIMEOUT);
        if (i == COM_TIMEOUT)
        {
            printf("readval: ...Device does not answer!\n");
            return -3;
        }
        else if (i == COM_DISCONNECT)
        {
            printf("readval: ...Device disconnected!\n");
            return -4;
        }

        switch(j)
        {
            case 1:
                if(i == SUCCESS)
                {
                    return val;
                }
                break;

            case 2:
            case 3:
            case 4:
                val = val * 256 + i;
                j--;
                break;

            case 256:
                j = i;
                break;

            case 257:
                if(i == FAIL) {
                    return -2;
                }
                else if(i == ANSWER) {
                    j = 256;
                }
                break;

            default:
                printf("\nError: readval, i = %i, j = %i, val = %li\n", i, j, val);
                return -2;
        }
    }
    return -1;
}

/*****************************************************************************
 *
 *      Read from serial port
 *
 ****************************************************************************/
int com_read (int       device,
              char      *pszIn,
              size_t    tLen)
{
    static long         ticks = 0;
    char    c;
    size_t valid = rbuf_valid;

#define TICK_MAX 10000

  for (ticks = 0; (valid <= 0) && (ticks < TICK_MAX); ticks++)
  {
    valid = RS232_PollComport(device, pszIn, tLen);
    if (valid > 0) {
      break;
    } else
      valid = 0;
  }
  if (ticks == TICK_MAX)
    return COM_TIMEOUT;

  if (valid > tLen) { /* more bytes read */
    rbuf_valid = valid - tLen;
    memcpy(readbuf, pszIn+tLen, rbuf_valid);
    valid = tLen;
  } else {
    rbuf_valid = 0;
  }
  return valid;
}

/**
 * Verify the controller
 */
int verifyflash (int           device,
                 char        * data,
                 unsigned long lastaddr,
                 bootInfo_t  * bInfo)
{
    unsigned char d1;
    unsigned long addr = 0;

    // Sending commands to MC
    sendcommand(device, VERIFY);

    if(com_getc(device, TIMEOUT) == BADCOMMAND)
    {
        printf("Verify not available\n");
        return 0;
    }
    printf( "Verify        : 0x00000 - 0x%05lX\n", lastaddr);

    do
    {
        d1 = data[addr];

        if ((d1 == ESCAPE) || (d1 == 0x13))
        {
            com_putc(device, ESCAPE);
            d1 += ESC_SHIFT;
        }
        if (addr % bInfo->blocksize)
            com_putc (device, d1);
        else
            com_putc (device, d1);

    } while (addr++ < lastaddr);

    printf("\nFinished.\n");


    com_putc(device, ESCAPE);
    com_putc(device, ESC_SHIFT); // A5,80 = End

    switch (com_getc (device, TIMEOUTP))
    {
        case SUCCESS:
            // o.k.
            break;
        case COM_DISCONNECT:
            printf("\n ---- Device disconnected ----");
            // FALLTHROUGH
        default:
        printf("\n ---------- Failed! ----------\n");
        return 3;
    }
    return 0;
}


/**
 * Flashes the controller
 */
int programflash (int           device,
                  char        * data,
                  unsigned long lastaddr,
                  bootInfo_t *  bInfo)
{
    unsigned long i;
    unsigned char d1;
    unsigned long addr = 0;

    // Sending commands to MC
    printf("Programming   : 0x00000 - 0x%05lX\n", lastaddr);
    sendcommand(device, PROGRAM);

    // Sending data to MC
    i = bInfo->buffsize;

    do
    {
        d1 = data[addr];

        if ((d1 == ESCAPE) || (d1 == 0x13))
        {
            com_putc(device, ESCAPE);
            d1 += ESC_SHIFT;
        }
        if (i % bInfo->blocksize)
            com_putc (device, d1);
        else
            com_putc (device, d1);

        if (--i == 0)
        {
            switch (com_getc (device, TIMEOUTP))
            {
                case CONTINUE:
                    // o.k.
                    break;
                case COM_DISCONNECT:
                    printf("\n ---- Device disconnected ----");
                    // FALLTHROUGH
                default:
                    printf("\n ---------- Failed! ----------\n");
                    return 2;
            }

            // set nr of bytes with next block
            i = bInfo->buffsize;
        }
    } while (addr++ < lastaddr);

    printf("\nCompleted\n");

    com_putc(device, ESCAPE);
    com_putc(device, ESC_SHIFT); // A5,80 = End

    switch (com_getc (device, TIMEOUTP))
    {
        case SUCCESS:
            // o.k.
            break;
        case COM_DISCONNECT:
            printf("\n ---- Device disconnected ----");
            // FALLTHROUGH
        default:
        printf("\n ---------- Failed! ----------\n");
        return 3;
    }
    return 0;
}



/**
 * Try to connect a device
 */
int connect_device ( int device,
                     const char *password )
{
    const char * ANIM_CHARS = "-\\|/";

    int state = 0;
    int val = 0;

    char passtring[32];

    // first 0x0d for autobaud, then password, then 0xff
    // for answer in one-line mode
    sprintf (passtring, "%c%s%c", 0x0d, password, 0xff);

    printf("Waiting for device...  ");

    while (running)
    {
        const char *s = passtring; //password;

        if (autoreset == AUTORESET)
        {
            if ((state & 0x0f) == 0x00)
                RS232_toggleDTR (device);
        }

        usleep (25000);     // just to slow animation...
        printf("\b%c", ANIM_CHARS[state++ & 3]);
        fflush(stdout);

        while ((val = *s++) != 0)
        {
            com_putc(device, val);

            val = com_getc(device, 0);

	    printf("val:%x\n", val);
            if (val == CONNECT)
            {
                printf ("\bconnected");

                // clear buffer from echo...
                while (com_getc(device, TIMEOUT) > 0);

                sendcommand( device, COMMAND );

                while (1)
                {
                    switch(com_getc(device, TIMEOUT))
                    {
                        case COM_DISCONNECT:
                            printf ("\nDevice disconnected.\n");
                            return 0;
                            break;
                        case COMMAND:
                            printf (" (one wire)");
                            break;
                        case SUCCESS:
                        case COM_TIMEOUT:
                            printf ("!\n");
                            return 1;
                    }
                }
            }
            else if (val == COM_DISCONNECT)
            {
                printf ("\nDevice disconnected.\n");
                return 0;
            }
        }
    }
    printf ("\nTerminated by user.\n");

    return 0;
}


/**
 * Checking CRC Support
 *
 * @return 2 if no crc support, 0 if crc supported, 1 fail, exit on timeout
 *
 * Sequence is:
 * send COMMAND CHECK_CRC  lobyte(crc) hibyte(crc)
 *      0xA5    0x06       0xnn        0xnn
 * wait for SUCCESS, FAIL, BADCOMMAND
 *          0xAA     0xAB  0xA7
 */
int check_crc(int device)
{
    int i;
    unsigned int crc1;

    sendcommand(device, CHECK_CRC);
    crc1 = crc;
    com_putc(device, crc1);
    com_putc(device, crc1 >> 8);

    i = com_getc(device, TIMEOUT);
    switch (i) {
        case SUCCESS:
            return 0;
        case BADCOMMAND:
            return 2;
        case FAIL:
            return 1;
        case COM_DISCONNECT:
            // FALLTHROUGH
        case COM_TIMEOUT:
            printf("check_crc: ...Device does not answer!\n\n");
            // FALLTHROUGH
        default:
            return i;
    }
}

/**
 * prints the device signature
 *
 * @return true on success; exit on error
 */
int read_info (int device,
               bootInfo_t *bInfo)
{
    long i, j;
    char s[256];
    FILE *fp;

    bInfo->crc_on = check_crc(device);
    if (bInfo->crc_on < 0)
        return (0);

    sendcommand(device, REVISION);

    i = readval(device);
    if(i < 0) {
        printf("Bootloader Version unknown (Fail)\n");
        bInfo->revision = -1;
    } else {
        printf("Bootloader    : V%lX.%lX\n", i>>8, i&0xFF);
        bInfo->revision = i;
    }

    sendcommand(device, SIGNATURE);

    i = readval(device);
    if (i < 0) {
        printf("Reading device SIGNATURE failed!\n\n");
        return (0);
    }
    bInfo->signature = i;

    // search locally...
    for (j = 0; j < (sizeof (avr_dev) / sizeof (avrdev_t)); j++) {
	if (i == avr_dev[j].id) {
	    strcpy (s, avr_dev[j].name);
	    break;
	  }
      }
    if (j == (sizeof (avr_dev) / sizeof (avrdev_t))) {
	sscanf ("(?)" , "%s", s);
	printf("File \"devices.txt\" not found!\n");
      }
    printf("Target        : %06lX %s\n", i, s);

    sendcommand(device, BUFFSIZE);

    i = readval(device);
    if (i < 0) {
        printf("Reading BUFFSIZE failed!\n\n");
        return (0);
    }
    bInfo->buffsize = i;

    printf("Buffer        : %ld Byte\n", i );

    sendcommand(device, USERFLASH);

    i = readval(device);
    if (i < 0) {
        printf("Reading FLASHSIZE failed!\n\n");
        return (0);
    }
    if( i > MAXFLASH) {
        printf("Device and flashsize do not match!\n");
        return (0);
    }
    bInfo->flashsize = i;

    printf("Size available: %ld Byte\n", i );

    if(bInfo->crc_on != 2) {
        bInfo->crc_on = check_crc(device);
        switch(bInfo->crc_on) {
            case 2:
                printf("No CRC support.\n");
                break;
            case 0:
                printf("CRC enabled and OK.\n");
                break;
            case 3:
                printf("CRC check failed!\n");
                break;
            default:
                printf("Checking CRC Error (%i)!\n", bInfo->crc_on);
                break;
        }
    } else {
        printf("No CRC support.\n\n");
    }

    return 1;
}//int read_info()



int prog_verify (int            mode,
                 int            baud,
                 int            block_size,
                 const char     *password,
                 const unsigned int device,
                 const char     *hexfile)
{
    char        *data = NULL;
    bootInfo_t  bootinfo;

    // last address in hexfile
    unsigned long last_addr = 0;

    // init bootinfo
    memset (&bootinfo, 0, sizeof (bootinfo));

    // set to maximum, is later in read_info corrected to the
    // size available in the controller...
    bootinfo.flashsize = MAXFLASH;
    bootinfo.blocksize = block_size;

    printf ("Now ");
    if (mode & AVR_CLEAN)
        printf ("erase, ");
    if (mode & AVR_PROGRAM)
        printf ("program, ");
    if (mode & AVR_VERIFY)
        printf ("verify, ");
    printf ("\b\b device.\n");

    printf("Port          : %s\n", comports[device]);
    printf("Baudrate      : %d\n", baud);

    if (mode & AVR_CLEAN) {
        data = malloc(MAXFLASH);

        if (data == NULL)
            printf("Memory allocation error, could not get %d bytes for flash-buffer!\n",
                   MAXFLASH);
        else
            memset (data, 0xff, MAXFLASH);

        last_addr = MAXFLASH - 1;
    } else {
        printf("File          : %s\n", hexfile);

        // read the file
        data = read_hexfile (hexfile, &last_addr);

        printf("Size          : %ld Bytes\n", last_addr + 1);
    }

    if (data == NULL) {
        printf ("ERROR: no buffer allocated and filled, exiting!\n");
        return (-1);
    }

    printf("-------------------------------------------------\n");

    // now start with target...
    if (connect_device (device, password)) {
        if (!read_info (device, &bootinfo)) {
            return (-3);
        }

        if (mode & AVR_CLEAN) {
            last_addr = bootinfo.flashsize - 1;
        }

        // now check if program fits into flash
        if ((mode & (AVR_PROGRAM | AVR_VERIFY)) &&
            (last_addr >= bootinfo.flashsize  )) {
            printf ("ERROR: Hex-file too large for target!\n"
                    "       (needs flash-size of %ld bytes, we have %ld bytes)\n",
                    last_addr + 1, bootinfo.flashsize);
            return (-2);
        }

        if (mode & AVR_PROGRAM) {
            if (programflash (device, data, last_addr, &bootinfo) == 0) {
                if ((bootinfo.crc_on != 2) && (check_crc(device) != 0))
                    printf("\n ---------- Programming failed (wrong CRC)! ----------\n\n");
                else if (mode & AVR_CLEAN)
                    printf("\n ++++++++++ Device successfully erased! ++++++++++\n\n");
                else
                    printf("\n ++++++++++ Device successfully programmed! ++++++++++\n\n");
            } else {
                printf("\n ---------- Programming failed! ----------\n\n");
                return (-5);
            }
        }
        if (mode & AVR_VERIFY) {
            if (verifyflash (device, data, last_addr, &bootinfo) == 0) {
                if ((bootinfo.crc_on != 2) && (check_crc(device) != 0))
                    printf("\n ---------- Verification failed (wrong CRC)! ----------\n\n");
                else
                    printf("\n ++++++++++ Device successfully verified! ++++++++++\n\n");
            } else
                printf("\n ---------- Verification failed! ----------\n\n");
        }

        if (!(mode & AVR_CLEAN))
            printf("...starting application\n\n");

        sendcommand(device, START);         //start application
        sendcommand(device, START);
    }
    free (data);

    return 0;
}

/**
 * Main, startup
 */
int main(int argc, char *argv[])
{
    // default values
    unsigned int baudid = 0;
    int device = 6;

    // Checking baudrate
    baudid = 9600;
    char pmode[]={'8','N','1',0};
    int retOpen = RS232_OpenComport(device, baudid, pmode);

    int mode = AVR_PROGRAM | AVR_VERIFY;
    prog_verify (mode, baudid, bsize, password, device, hexfile);

    RS232_CloseComport(device);
    return 0;
}

/* end of file */

    #define	INH32M_HEADER	":020000040000FA"

    // Array access operator
    value_type& hex_data::operator[](address_type address)
    {
	// Start at the end of the list and find the first (last) block with an address
	//  less than addr
	reverse_iterator i = blocks.rbegin();
	while( i != blocks.rend() )
	{
	    if( i->first <= address )
	    {
		// Use the block if address is interior or adjacent to the block
		if( (address - i->first) <= i->second.size() )
		    return i->second[address - i->first];
		break;
	    }
	    ++i;
	}
	return blocks[address][0];
    }

    // Return the value at address, or _fill if not set
    value_type hex_data::get(address_type address)
    {
	// Start at the end of the list and find the first (last) block with an address
	//  less than addr
	reverse_iterator i = blocks.rbegin();
	while( i != blocks.rend() )
	{
	    if( i->first <= address )
	    {
		// Use the block if address is interior to the block
		if( (address - i->first) < i->second.size() )
		    return i->second[address - i->first];
		break;
	    }
	    ++i;
	}
	return _fill;
    }

    // Set the value at address or create a new element using value
    void hex_data::set(address_type address, value_type value)
    {
	if( value == fill() )	// Handle fill values
	{
	    erase(address);	// If the address is already set, erase it
	    return;
	}

	// Start at the end of the list and find the first (last) block with an address
	//  less than addr
	reverse_iterator i = blocks.rbegin();
	while( i != blocks.rend() )
	{
	    if( i->first <= address )
	    {
		// Use the block if address is interior or adjacent to the block
		const address_type index = address - i->first;
		if( index < i->second.size() )
		{
		    i->second[index] = value;
		    return;
		}
		else if( index == i->second.size() )
		{
		    i->second.push_back(value);
		    return;
		}
		break;
	    }
	    ++i;
	}
	blocks[address].push_back(value);	// Otherwise create a new block
    }

    // Merge adjacent blocks
    void hex_data::compact()
    {
	iterator previous = blocks.begin();
	iterator i = previous;

	for(++i; i != blocks.end(); ++i)
	{
	    if( (previous->first + previous->second.size()) == i->first )
	    {
		previous->second.insert(previous->second.end(), i->second.begin(), i->second.end());
		blocks.erase(i);
		i = previous;
	    }
	    previous = i;
	}
    }

    // Delete all allocated memory
    void hex_data::clear()
    {
	_fill = 0;
	format = HEX_FORMAT_INHX8M;
	linear_addr_rec = false;
	segment_addr_rec = false;
	blocks.clear();
    }

    // Erase a single element at the given address
    void hex_data::erase(address_type address)
    {
	for(iterator i=blocks.begin(); i!=blocks.end(); ++i)
	{
	    // The blocks are sorted, so if the byte to be deleted is
	    //  before the block it must be a blank address that's either
	    //  before the first block or after any previous blocks.
	    if( address < i->first )
		break;
	    // Ignore the block if address is past the end of the block
	    const address_type ope = i->first + i->second.size();
	    if( address >= ope )
		continue;
	    // address is now guaranteed to be >= i->first and < ope
	    // Copy trailing portion of the old block to a new block
	    if( (ope - address) > 1 )
	    {
		const address_type index = address-i->first+1;
		blocks[address+1].assign(i->second.begin()+index, i->second.end());
	    }
	    // Truncate or delete old block
	    const address_type size = address - i->first;
	    if( size )
		i->second.resize(size);
	    else
		blocks.erase(i);
	    break;
	}
    }

    // Erase [first, last]
    void hex_data::erase(address_type first, address_type last)
    {
	if( first > last )
	    std::swap(first, last);

	for(iterator i=blocks.begin(); (i!=blocks.end()) && (first<=last); ++i)
	{
	    const address_type ope = i->first + i->second.size();
	    if( first >= ope )	// Ignore all blocks with addresses < first
		continue;
	    // The blocks are sorted, so if the first byte to be deleted is
	    //  before the block it must be a blank address that's either
	    //  before the first block or after any previous blocks.
	    if( first < i->first )
	    {
		if( last < i->first )	// If the entire range is before the
		    return;		//  block there's nothing left to do
		first = i->first;   // Advance to the next non-blank address
	    }
	    // first is now guaranteed to be >= i->first and < ope
	    if( last < ope )	// Entire range is interior
	    {
		// Copy trailing portion of the old block to a new block
		if( (ope - last) > 1 )
		{
		    const address_type index = last-i->first+1;
		    blocks[last+1].assign(i->second.begin()+index, i->second.end());
		}
		// Truncate or delete old block
		const address_type size = first - i->first;
		if( size )
		    i->second.resize(size);
		else
		    blocks.erase(i);
		return;
	    }
	    else	// Truncate block
	    {
		const address_type size = first - i->first;
		if( size )
		    i->second.resize(size);
		else
		    blocks.erase(i--);
		first = ope;
	    }
	}
    }

    hex_data::size_type hex_data::size()
    {
	size_type s=0;

	for(iterator i=blocks.begin(); i!=blocks.end(); ++i)
	    s += i->second.size();

	return s;
    }

    // Returns the number of populated elements with addresses less than addr
    hex_data::size_type hex_data::size_below_addr(address_type addr)
    {
	size_type s=0;

	for(iterator i=blocks.begin(); i!=blocks.end(); ++i)
	{
	    if( (i->first + i->second.size()) < addr)
		s += i->second.size();
	    else if( i->first < addr )
		s += addr - i->first;
	}

	return s;
    }

    // number of words in [lo, hi)
    hex_data::size_type hex_data::size_in_range(address_type lo, address_type hi)
    {
	size_type s=0;

	for(iterator i=blocks.begin(); i!=blocks.end(); ++i)
	{
	    if( i->first < lo )
	    {
		const size_type a = i->first + i->second.size();
		if( a >= lo )
		    s += a  - lo;
	    }
	    else
	    {
		if( (i->first + i->second.size()) < hi)
		    s += i->second.size();
		else if( i->first < hi )
		    s += hi - i->first;
	    }
	}

	return s;
    }

    // Return the max address of all of the set words with addresses less than or equal to hi
    address_type hex_data::max_addr_below(address_type hi)
    {
	address_type s=0;

	for(iterator i=blocks.begin(); i!=blocks.end(); ++i)
	{
	    if( i->first <= hi)
	    {
		const address_type a = i->first + i->second.size() - 1;	//Max address of this block
		if( a > s )
		    s = a;
	    }
	}
	if( s > hi )
	    return hi;
	else
	    return s;
    }

    // Lowest address
    address_type hex_data::min_address() const
    {
	return blocks.begin()->first;
    }

    // Highest address
    address_type hex_data::max_address() const
    {
	return blocks.rbegin()->first + blocks.rbegin()->second.size() - 1;
    }

    //Return true if an element exists at addr
    bool hex_data::is_set(address_type addr)
    {
	// Start at the end of the list and find the first (last) block with an address
	//  less than addr
	reverse_iterator i = blocks.rbegin();
	while( (i!=blocks.rend()) && (i->first > addr))
	    ++i;

	if( (addr - i->first) > i->second.size() )
	    return false;
	else
	    return true;
    }

    // Load from a file
    void hex_data::load(const std::string &path)
    {
	std::ifstream f(path.c_str());
	read(f);
    }

    // Convert a string from hex to binary and append it to a block
    uint8_t hex2binary(hex_data::data_container& to, std::string& from)
    {
	value_type    sum = 0, value;
	uint8_t character;
	bool first = true;
	std::string::iterator i = from.begin();

	while( i != from.end() )
	{
	    character = *i;

	    if( (character >= '0') && (character <= '9') )
		character -= '0';
	    else if( (character >= 'A') && (character <= 'Z') )
		character -= 'A' - 10;
	    else if( (character >= 'a') && (character <= 'z') )
		character -= 'a' - 10;
	    else
		break;	// Bad character

	    if( first )
		value = character << 4;
	    else
	    {
		value |= character;
		to.push_back(value);
		sum += value;
	    }

	    first = !first;
	    ++i;
	}

	return sum;
    }

    // Read data from an input stream
    void hex_data::read(std::istream &s)
    {
	address_type   address;
	address_type   linear_address(0);
	std::string line;
	data_container buffer;

	while( (s.get() == ':') && s.good() )
	{
	    getline(s, line);		    // Read the whole line
	    if( line.size() <= 10 )	    // Ignore truncated lines
		break;
	    buffer.clear();
	    buffer.reserve(line.size()/2);  // Pre-allocate
	    if( hex2binary(buffer, line) )  // Ignore lines with bad checksums
		break;

	    address = buffer[1];
	    address = (address << 8) | buffer[2];
	    unsigned length = buffer[0];
	    const unsigned type = buffer[3];
	    value_type* data = &buffer[4];

	    switch(type)
	    {
		case 0: 	//Data block
		{
		    address += linear_address;
		    iterator i = blocks.begin();
		    for(; i != blocks.end(); ++i )  // Find a block that includes address
		    {
			address_type num = 0;
			// If the start of the new block is interior to an existing block...
			if( (i->first <= address) && ( (i->first + i->second.size()) > address) )
			{
			    // Store the portion of the new block that overlaps the existing block
			    const size_type index = address - i->first;
			    num = i->second.size() - index;
			    if( num > length )
				num = length;
			    std::copy(data, data+num, &(i->second[index]));
			}
			// If the end of the new block is interior to an existing block...
			if( (address < i->first) && ((address + length) > i->first) )
			{
			    // Create a new block for the non-overlapping portion
			    num = i->first - address;
			    if( num > length )
				num = length;
			    blocks[address].assign(data, data+num);
			}
			length -= num;
			address += num;
			data += num;
			// Bail out early if there's nothing left to do
			if( 0 == length )
			    break;
		    }
		    // Handle any leftover bytes
		    if( length )
			blocks[address].assign(data, data+length);
		    break;
		}
		case 1: break;	// Ignore EOF record
		case 2:		// Segment address record (INHX32)
		    segment_addr_rec = true;
		    break;
		case 4:		// Linear address record (INHX32)
		    if( (0 == address) && (2 == length) )
		    {
			linear_address = buffer[4];
			linear_address = (linear_address << 8) | buffer[5];
			linear_address <<= 16;
			linear_addr_rec = true;
		    }
		    break;
	    }
	}
    }

    // Write all data to a file
    void hex_data::write(const char *path)
    {
	std::ofstream	ofs(path);
	if( !ofs )	// Bail out on bad files
	    return;
	write(ofs);
	ofs.close();
    }

    // Write all data to an output stream
    void hex_data::write(std::ostream &os)
    {
	uint8_t	checksum;
	uint16_t	linear_address(0);

	if( !os )	    // Bail out on bad streams
	    return;

	os.setf(std::ios::hex, std::ios::basefield);	//Set the stream to ouput hex instead of decimal
	os.setf(std::ios::uppercase);			//Use uppercase hex notation
	os.fill('0');					//Pad with zeroes

	//If we already know that this is an INHX32M file, start with a segment address record
	//	otherwise check all of the blocks just to make sure
	if( linear_addr_rec )
	{
	    os << INH32M_HEADER << std::endl;
	}
	else
	{
	    for(iterator i=blocks.begin(); i!=blocks.end(); i++)
	    {
		if(i->first > 0xFFFF)	//Check the upper 16 bits
		{
		    linear_addr_rec = true;
		    os << INH32M_HEADER << std::endl;
		    break;	//Only need to find one
		}
	    }
	}

	for(iterator i=blocks.begin(); i!=blocks.end(); i++)
	{
	    // Check upper 16 bits of the block address for non-zero,
	    //  which indicates that a segment address record is needed
	    if( i->first > 0xFFFF )
	    {
		const uint16_t addr(i->first >> 16);
		//Has a record for this segment already been emitted?
		if( addr != linear_address )
		{
		    //Emit a new segment address record
		    os << ":02000004";
		    os.width(4);
		    os << addr;	//Address
		    // Create a checksum for the linear address record
		    checksum = 0x06 + addr + (addr >> 8);
		    checksum = 0x01 + ~checksum;
		    os.width(2);
		    // OSX (or maybe GCC), seems unable to handle uint8_t
		    //  arguments to a stream
		    os << static_cast<uint16_t>(checksum);	// Checksum byte
		    os << std::endl;
		    linear_address = addr;
		}
	    }
	    checksum = 0;
	    os << ':';	//Every line begins with ':'
	    os.width(2);
	    os << i->second.size();				//Length
	    checksum += i->second.size();
	    os.width(4);
	    os << static_cast<uint16_t>(i->first);		//Address
	    checksum += static_cast<uint8_t>(i->first);		// Low byte
	    checksum += static_cast<uint8_t>(i->first >> 8);	// High byte
	    os << "00";											//Record type
	    for(unsigned j=0; j<i->second.size(); ++j)	//Store the data bytes, LSB first, ASCII HEX
	    {
		os.width(2);
		// OSX (or maybe GCC), seems unable to handle uint8_t
		//  arguments to a stream
		os << static_cast<uint16_t>(i->second[j]);
		checksum += i->second[j];
	    }
	    checksum = 0x01 + ~checksum;
	    os.width(2);
	    // OSX (or maybe GCC), seems unable to handle uint8_t arguments to a stream
	    os << static_cast<uint16_t>(checksum);	// Checksum byte
	    os << std::endl;
	}
	os << ":00000001FF\n";			//EOF marker
    }

    // Make things pretty
    //  Truncate blocks to a given length as needed
    void hex_data::tidy(hex_data::size_type length)
    {
	for(iterator i=blocks.begin(); i!=blocks.end(); i++)
	{
	    if(i->second.size() > length)	//If the block is too long...
	    {
		//Make an interator that points to the first element to copy out of i->second
		data_container::iterator k(i->second.begin());
		advance(k, length);

		// Assign the extra elements to a new block and truncate the original
		blocks[i->first + length].assign(k, i->second.end());
		i->second.erase(k, i->second.end());
	    }
	}
    }

    //Compare two sets of hex data
    //	Return true if every word in hex1 has a corresponding, and equivalent, word in hex2
    bool compare(hex_data& hex1, hex_data& hex2, value_type mask, address_type begin, address_type end)
    {
	//Walk block list from hex1
	for( hex_data::iterator i = hex1.begin(); i != hex1.end(); ++i )
	{
	    //Walk the block
	    address_type addr(i->first);
	    for( hex_data::data_container::iterator j = i->second.begin(); j != i->second.end(); ++j, ++addr)
	    {
		if( (addr < begin) || (addr > end) )
		    continue;

		//Compare both sides through the given mask
		if( ((*j) & mask) != (hex2.get(addr) & mask) )
		    return false;
	    }
	}
	return true;
    }
