//*************************************************************************
//          protocol definitions
//-------------------------------------------------------------------------
#define  COMMAND     0xA5        // Command sequence start
#define  ESCAPE      COMMAND

#define  CONNECT     0xA6        // connection established
#define  BADCOMMAND  0xA7        // command not supported
#define  ANSWER      0xA8        // followed by length byte
#define  CONTINUE    0xA9
#define  SUCCESS     0xAA
#define  FAIL        0xAB

#define  ESC_SHIFT   0x80        // offset escape char

#define  PROGEND     0xFF        // A5, FF
#define  REVISION    0           // get bootloader revision
#define  BUFFSIZE    1           // get buffer size
#define  SIGNATURE   2           // get target signature
#define  USERFLASH   3           // get user flash size
#define  PROGRAM     4           // program flash
#define  START       5           // start application
#define  CHECK_CRC   6           // CRC o.k.
#define  VERIFY      7           // Verify

#define  MAXFLASH 0x40000        // max flash size (256kB)

//-------------------------------------------------------------------------
//          APICALL definitions
//-------------------------------------------------------------------------
#define  API_PROG_PAGE  0x81     // copy one Page from SRAM to Flash

#define  API_SUCCESS    0x80     // success
#define  API_ERR_FUNC   0xF0     // function not supported
#define  API_ERR_RANGE  0xF1     // address inside bootloader
#define  API_ERR_PAGE   0xF2     // address not page aligned
//-------------------------------------------------------------------------

/*

Bootloader Protocol:

- The AVR is set by Power On Reset in.

- After about 65ms he goes for about 300ms in the bootloader
  and then starts the Application


1. Enabling the bootloader:

Send the password "Peda" 0xFF
Repeat until CONNECT received

Send COMMAND
Answer: SUCCESS

For baud rate, the password must contain a string, in the
contained 1 * Bit 0 followed by 4 * Bit 0th
The protocol characters (CONNECT) may not be included.


2. Recognition 1-wire mode:

It is checked whether the first character of the password to come back (echo).
The CONNECT is sent to the starting edge of 0xFF.

Log in Eindrahtmodus:
In the PC software, the bytes sent are counted for a command.
Only when the same number was received bytes, the following
Bytes counted in response.
Before that the bytes are ignored (just counted).


3. Query bootloader version:

Send: COMMAND, REVISION
Answer: ANSWER, 0x03, Revision_high, Revission_low, SUCCESS


4. Query Buffer Size:

Send: COMMAND, BUFFSIZE
Answer: ANSWER, 0x03, Buffersize_high, buffersize_low, SUCCESS


5. Query User flash:

Send: COMMAND, USER FLASH
Answer: ANSWER, 0x04, Flash_high, Flash_mid, Flash_low, SUCCESS


6. Program:

Send COMMAND, PROGRAM

Send bytes to buffer size reaches
Answer: CONTINUE
Repeat until all the bytes programmed

if byte == ESCAPE: Send ESCAPE (ESC + ESC_SHIFT)
if byte == 0x13: Send ESCAPE (0x13 + ESC_SHIFT)

End Programming: Send ESCAPE ESC_SHIFT
Answer: SUCCESS


7. Verify

Send COMMAND, VERIFY
Send bytes
if byte == ESCAPE: Send ESCAPE (ESC + ESC_SHIFT)

End Programming: Send ESCAPE ESC_SHIFT
Answer: SUCCESS - Verify o.k.
           FAIL - Verfy failed


8. Start application

Send COMMAND, START
Answer: No

9. CRC:

Send: COMMAND, CHECK_CRC, CRC_low, CRC_high
Answer: SUCCESS - CRC o.k.
           FAIL - CRC failed

It is the 16-bit CRC calculated with the polynomial 0xA001
After connection the command is sent, but not evaluated,
so that the AVR can start its CRC to 0.
After interrogation of the information, the CRC is 1.mal tested.
If then already carried out an error, is not programmed
(Old program is not destroyed).


10. error

Unknown commands are answered with BADCOMMAND.
For example, when CRC or Verify are excluded in ATtiny.
************************************************** ************************
*/

/* --- end of file --- */

