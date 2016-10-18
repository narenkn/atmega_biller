#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <array>

#define TOSTRING(X) #X
#define STRINGFY(x) TOSTRING(x)


// EssEss replacement 7760860615
#include "gsBillerUtils.h"
#include "../../default/billmc32.inc"
#include "../fastboot-2.9/converted/protocol.h"
#include "rs232.c"

using namespace std;

typedef struct
{
  uint32_t        id;
  const char      *name;
  uint32_t        flashSize;
} avrdev_t;
const avrdev_t avr_dev[] = {
  { 0x1e9502, "ATmega32", 0x7C00 },
};

/**
 * Calculate the new CRC sum
 */
inline void
gsBillerFbUtils::calcCrc(uint8_t ch)
{
  crc ^= ch;
  for ( auto i=0; i<8; i++ )
    crc = (crc >> 1) ^ ((crc & 1) ? 0xA001 : 0 );
}

void
gsBillerFbUtils::putChar(uint8_t ch)
{
  //  cout << "Sending 0x" << hex << (uint32_t) ch << endl;
  auto ret = RS232_SendByte(devId, ch);
  if (0 != ret)
    throw rErrSend;
  calcCrc( ch ); // calculate transmit CRC
  RS232_flushTX(devId);
}

/**
 * Sending a command
 */
void
gsBillerFbUtils::command(uint8_t cmd)
{
  auto ret = RS232_SendByte(devId, COMMAND);
  if (0 != ret)
    throw rErrSend;
  calcCrc( COMMAND ); // calculate transmit CRC
  //  cout << "Sending Cmd 0x" << hex << (uint32_t) COMMAND <<
  //    " 0x" << (uint32_t) cmd << endl;
  ret = RS232_SendByte(devId, cmd);
  if (0 != ret)
    throw rErrSend;
  calcCrc( cmd ); // calculate transmit CRC
  RS232_flushTX(devId);
}

#if 0
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
#endif

/**
 * Get status of devId. It might happen that the devId disappears (e.g.
 * due to a USB-serial unplug).
 *
 * @return 1 if devId seems ok, 0 if it seems not available
 */
void
gsBillerFbUtils::updDeviceStatus()
{
  status =
    (0 == GetCommModemStatus(Cport[devId], (LPDWORD)((void *)&status)))?
    CONNECT : ~CONNECT;
}

/**
 * Receives one char or -1 if timeout
 * timeout in 10th of seconds
 */
uint8_t
gsBillerFbUtils::getChar(uint32_t maxTicks)
{
  uint32_t  ticks = 0;
  uint8_t   ret;

  for (ticks = 0; (rbufValid <= 0) && (ticks < maxTicks); ticks++) {
    rbufValid = RS232_PollComport(devId, readBuf, RBUF_SIZE);
    if (rbufValid > 0) {
      break;
    } else
      rbufValid = 0;
    Sleep(1);
  }
  if (ticks >= maxTicks)
    throw rErrTimeout;

  rbufValid--;
  ret = readBuf[0];
  memcpy(readBuf, readBuf+1, rbufValid*sizeof(uint8_t));

  //  cout << "Obtained 0x" << hex << (uint32_t) ret << endl;
  return ret;
}

/**
 * Reads a value from bootloader
 */
uint32_t
gsBillerFbUtils::readInt32()
{
  uint8_t i;
  uint32_t ret;

  while(1) {
    ret = 0;
    i = getChar();
    if (i != ANSWER) continue;
    for (uint32_t j = getChar(); j>1; j--) {
      i = getChar();
      ret <<= 8;
      ret |= i;
    }
    i = getChar();
    if (SUCCESS == i)
      break;
    else
      throw rErrHost;
  }

  return ret;
}

void
gsBillerFbUtils::iFlush()
{
  do {
    rbufValid = RS232_PollComport(devId, readBuf, RBUF_SIZE);
  } while (rbufValid > 0);
  rbufValid = 0;
}

/*****************************************************************************
 *
 *      Read from serial port
 *
 ****************************************************************************/
uint16_t
gsBillerFbUtils::read ( uint8_t*   pszIn,
			uint32_t   tLen)
{
  auto valid = rbufValid;
  uint32_t ticks;

  if (valid)
    memcpy(pszIn, readBuf, valid);

  for (ticks = 0; (valid <= tLen) && (ticks < TICK_MAX);
       ticks++) {
    auto nr = RS232_PollComport(devId, pszIn+valid, tLen-valid);
    if (nr > 0) {
      ticks = 0;
      valid += nr;
    } else
      valid = 0;
  }
  if (ticks == TICK_MAX)
    throw rErrTimeout;

  if (valid > tLen) { /* more bytes read */
    rbufValid = valid - tLen;
    memcpy(readBuf, pszIn+tLen, rbufValid);
    valid = tLen;
  } else {
    rbufValid = 0;
  }

  return valid;
}

/**
 * Verify the controller
 */
void
gsBillerFbUtils::progOrVerifyFlash(uint8_t cmd)
{
  /* Sending commands to MC */
  command(cmd);

  for (uint32_t addr=0; ; addr+=SPM_PAGESIZE) {
    if (hexFile.end() == hexFile.find(addr))
      break;
    const page_t &page = hexFile[addr];
    for (const value_t &data: page) {
      uint8_t d1 = data;
      if ((d1 == ESCAPE) || (d1 == 0x13)) {
	putChar(ESCAPE);
	d1 += ESC_SHIFT;
      }
      putChar(d1);
    }

    /* made sure buffSize == SPM_PAGESIZE, wait for continue */
    if (CONTINUE != getChar(500))
      throw rErrDisconnect;

    cout << ".";
  }
  cout << endl;

  putChar(ESCAPE);
  putChar(ESC_SHIFT); // A5,80 = End

  if (SUCCESS != getChar())
    throw rErrRspFail;
}

/**
 * Try to connect a devId
 */
void
gsBillerFbUtils::connectDevice()
{
  static const char pMode[]="8N1";

  int retOpen = RS232_OpenComport(devId, baud, pMode);
  if (0 != retOpen)
    throw rErrNoCom;

  // pointer to password...
  // following characters are needed for autobaud
  // 0x0A - LF,  0x0B - VT,  0x0D - CR,  0x0F - SI
  // 0x21 - '!', 0x43 - 'C', 0x61 - 'a', 0x85, 0x87
  // 0xC3 - 'A~',0xE1 - 'a´' - ISO8859-1
  // first 0x0d for autobaud, then password, then 0xff
  // for answer in one-line mode
  const char passtring[] = "\x0DGurU\xFF";

  uint8_t val, resp = (uint8_t)~CONNECT;
  for (auto ticks = 0; (resp != CONNECT) && (ticks < TICK_MAX);
       ticks++) {
      const char *s = passtring;

      if (autoreset == AUTORESET) {
	RS232_toggleDTR (devId);
      }
      Sleep(5);

      while ((val = *s++) != 0) {
	putChar(val);
      }
      try {
	resp = getChar();
      } catch (const char *err) {
	continue;
      }

      if (CONNECT == resp)
	return;
  }

  throw rErrTimeout;
}

void
gsBillerFbUtils::disconnectDevice()
{
  RS232_CloseComport(devId);
}

/**
 * Checking CRC Support
 *
 * @return 2 if no crc support, 0 if crc supported, 1 fail, exit on timeout
 *
 * Sequence is:
 * send COMMAND CHECKCRC  lobyte(crc) hibyte(crc)
 *      0xA5    0x06       0xnn        0xnn
 * wait for SUCCESS, FAIL, BADCOMMAND
 *          0xAA     0xAB  0xA7
 */
uint8_t
gsBillerFbUtils::checkCrc()
{
  uint16_t crc1;
  command(CHECK_CRC);
  crc1 = crc;

  putChar(crc1);
  putChar(crc1 >> 8);

  return getChar();
}

/**
 * prints the devId signature
 */
void
gsBillerFbUtils::readInfo()
{
  uint8_t crcStat;

  crcStat = checkCrc();
  crcOn = (SUCCESS == crcStat) ? true : false;
  cout << "CRC " << (crcOn ? "ON" : "OFF") << endl;

  command(REVISION);
  revision = readInt32();

  command(SIGNATURE);
  signature = readInt32();

  /* search locally... */
  uint32_t j;
  for (j = 0; j < (sizeof (avr_dev) / sizeof (avrdev_t)); j++) {
    if (signature == avr_dev[j].id) {
      break;
    }
  }
  if ((j >= (sizeof (avr_dev) / sizeof (avrdev_t))) ||
      (0 != j)) {
    throw rErrDevUnknown;
  }

  command(BUFFSIZE);
  bufSize = readInt32();
  if (0 != (bufSize%SPM_PAGESIZE))
    throw rErrBufSize;

  command(USERFLASH);
  if (avr_dev[j].flashSize != readInt32())
    throw rErrFlashSize;
}

gsBillerFbUtils biller(6);

void
readVerify()
{
  // now start with target...
  biller.connectDevice();
  cout << "Device connected " << endl;
  biller.iFlush();
  biller.readInfo();
  cout << "ReadInfo completed " << endl;
  biller.iFlush();

  biller.progOrVerifyFlash(PROGRAM);
  cout << "Program Completed" << endl;
  if (biller.crcOn)
    if (SUCCESS != biller.checkCrc())
      throw rErrCrc;
    else
      cout << "Prog CRC Success" << endl;

  cout << "Verify Started" << endl;
  biller.progOrVerifyFlash(VERIFY);
  cout << "Verify Completed" << endl;
  if (biller.crcOn)
    if (SUCCESS != biller.checkCrc())
      throw rErrCrc;
    else
      cout << "Verify CRC Success" << endl;

  /* start application */
  biller.command(START);
}

/**
 * Main, startup
 */
int
main(int argc, char *argv[])
{
  try {
    readVerify();
  } catch(rs232_error_t err) {
    cerr << "Error: " << (uint32_t) err << endl;
  } catch (uint32_t err) {
    cerr << "Error uint32_t: " << (uint32_t) err << endl;
  }

  return 0;
}
