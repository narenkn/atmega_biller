#include "i2c.h"

uint8_t i2c_ymd[3], i2c_hm[3];

#define NIBBLE_PACK(A, B) ((A<<4)|B)
#define EEPROM_SIZE ((1<<16)<<EEPROM_MAX_DEVICES_LOGN2)
uint8_t i2c_bytes[EEPROM_SIZE];

#define I2C_EEPROM_DIRECT_ASSIGN(addr, val) \
  i2c_bytes[addr] = val

extern void LCD_end(void);

void
i2c_init(void)
{
  i2c_ymd[0] = NIBBLE_PACK(0, 1);
  i2c_ymd[1] = NIBBLE_PACK(0, 1);
  i2c_ymd[2] = NIBBLE_PACK(1, 4);

  uint32_t ui1;
  for (ui1=0; ui1<EEPROM_SIZE; ui1++)
    i2c_bytes[ui1] = rand();
}

uint8_t
i2c_start(void)
{
  assert(0);
  return 0;
}

uint8_t
i2c_repeatStart(void)
{
  assert(0);
  return 0;
}

uint8_t
i2c_sendAddress(uint8_t address)
{
  assert(0);
  return 0;
}

uint8_t
i2c_sendData(uint8_t address)
{
  assert(0);
  return 0;
}

uint8_t
i2c_receiveData_ACK(void)
{
  assert(0);
  return 0;
}

uint8_t
i2c_receiveData_NACK(void)
{
  assert(0);
  return 0;
}

void
i2c_stop(void)
{
  assert(0);
}

#define ee24xx_write_page ee24xx_write_bytes

uint16_t
ee24xx_write_bytes(uint16_t addr, uint8_t *data, uint16_t num_bytes)
{
  uint16_t n_bytes;
  uint32_t addr_t = addr;

  addr_t <<= 2;
  assert(0 == (num_bytes&0x3));

  for (n_bytes=0; n_bytes<num_bytes; n_bytes++) {
    assert((addr_t+n_bytes) < EEPROM_SIZE);
    i2c_bytes[addr_t+n_bytes] = (NULL == data) ? 0 : data[n_bytes];
    //if (NULL != data)
    //printf("i2c[%x] = %x\n", (addr_t+n_bytes), data[n_bytes]);
  }

  return num_bytes;
}

uint16_t
ee24xx_read_bytes(uint16_t addr, uint8_t *data, uint16_t num_bytes)
{
  uint16_t n_bytes;
  uint32_t addr_t = addr;

  addr_t <<= 2;
  assert(0 == (num_bytes&0x3));

  for (n_bytes=0; n_bytes<num_bytes; n_bytes++) {
    if ((addr_t+n_bytes) >= EEPROM_SIZE) {
      LCD_end();
      if ((addr_t+n_bytes) >= EEPROM_SIZE)
	assert((addr_t+n_bytes) < EEPROM_SIZE);
    }
    data[n_bytes] = i2c_bytes[addr_t+n_bytes];
  }

  return num_bytes;
}

#undef timerDateSet
#undef timerDateGet
#undef timerTimeSet
#undef timerTimeGet

void
timerDateSet(uint8_t year, uint8_t month, uint8_t date)
{
  i2c_ymd[0] = date;
  i2c_ymd[1] = month;
  i2c_ymd[2] = year;
}

void
timerDateGet(uint8_t *ymd)
{
  ymd[0] = i2c_ymd[0];
  ymd[1] = i2c_ymd[1];
  ymd[2] = i2c_ymd[2];
}

void
timerTimeSet(uint8_t hour, uint8_t min)
{
  i2c_hm[1] = min;
  i2c_hm[0] = hour;
}

void
timerTimeGet(uint8_t *hm)
{
  hm[0] = i2c_hm[0];
  hm[1] = i2c_hm[1];
  hm[2] = i2c_hm[2];
}

//******************************************************************
//Function to get RTC date & time in FAT32 format
//  Return format : Year[31:25], Month[24:21], Date[20:16]
//                  Hour[15:11], Min[10:5], Sec[4:0]
//******************************************************************  
uint32_t
get_fattime (void)
{
  uint8_t buf[3];
  uint32_t dtFat;

  /* Process date */
  timerDateGet(buf);
  dtFat = buf[2];
  dtFat <<= 4;
  dtFat |= buf[1];
  dtFat <<= 5;
  dtFat |= buf[0];

  /* Process time */
  timerTimeGet(buf);
  dtFat <<= 5;
  dtFat |= buf[0];
  dtFat <<= 6;
  dtFat |= buf[1];
  /* FAT32 fromat accepts dates with 2sec resolution
     (e.g. value 5 => 10sec) */
  dtFat <<= 5;
  dtFat |= buf[2]>>1;

  return dtFat;
}
