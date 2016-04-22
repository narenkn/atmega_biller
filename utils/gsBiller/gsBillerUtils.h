#ifndef GsBillerUtils
#define GsBillerUtils

typedef enum {
  rErrSend=1, rErrTimeout, rErrHost,
  rErrBadCommand, rErrDisconnect,
  rErrRspFail, rErrNoCom, rErrBufSize,
  rErrFlashSize, rErrCrc, rErrDevUnknown
} rs232_error_t;
struct gsBillerFbUtils
{
  enum { RBUF_SIZE=800, TICK_MAX=10 };
  enum { NO_AUTORESET, AUTORESET } autoreset;

  uint8_t  status;
  uint8_t  readBuf[RBUF_SIZE];
  uint16_t rbufValid;
  uint16_t devId;

  uint32_t  baud;
  uint16_t  bufSize;

  uint32_t revision;
  uint32_t signature;
  uint16_t crc;
  bool     crcOn;

gsBillerFbUtils(uint16_t di) : rbufValid(0),
    status(0), crc(0), crcOn(true),
    baud(9600), autoreset(AUTORESET), devId(di)
  {
  }

  void connectDevice ();
  void disconnectDevice();

  inline void calcCrc(uint8_t ch);

  uint8_t getChar(uint32_t maxTicks=TICK_MAX);
  uint32_t readInt32();
  uint16_t read(uint8_t* pszIn, uint32_t   tLen);
  void iFlush();

  void putChar(uint8_t ch);
  void command(uint8_t);

  /* public routines */
  void updDeviceStatus();
  void readInfo();
  void progOrVerifyFlash(uint8_t cmd);
  uint8_t checkCrc();
};

#endif
