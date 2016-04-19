#ifndef GsBillerUtils
#define GsBillerUtils

struct gsBillerFbUtils
{
  enum { RBUF_SIZE=800, TICK_MAX=1000 };
  enum { NO_AUTORESET, AUTORESET } autoreset;

  uint8_t  status;
  uint8_t  readBuf[RBUF_SIZE];
  uint16_t rbufValid;
  uint16_t devId;

  uint32_t  baud;

  uint32_t revision;
  uint32_t signature;
  uint32_t crc;
  bool     crcOn;

gsBillerFbUtils(uint16_t di) : rbufValid(0),
    status(0), crc(0), crcOn(true),
    baud(38400), autoreset(AUTORESET), devId(di)
  {
  }

  void connectDevice ();
  void disconnectDevice();

  inline void calcCrc(uint8_t ch);
  void putChar(uint8_t ch);
  uint8_t getChar();
  void command(uint8_t);
  void clearInBuf();
  void updDeviceStatus();
  uint32_t readInt32();
  uint16_t read(uint8_t* pszIn, uint32_t   tLen);
  void readInfo();
  void progOrVerifyFlash(uint8_t cmd);
  uint8_t checkCrc();
};

#endif
