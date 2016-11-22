#include "i2c.h"

date_t i2c_date;
s_time_t i2c_time;

#define NIBBLE_PACK(A, B) ((A<<4)|B)
#define EEPROM_SIZE ((1<<16)<<EEPROM_MAX_DEVICES_LOGN2)
uint8_t i2c_bytes[EEPROM_SIZE];

#define I2C_EEPROM_DIRECT_ASSIGN(addr, val) \
  i2c_bytes[addr] = val

void
i2c_init(void)
{
  i2c_date.day = 1;
  i2c_date.month = 10;
  i2c_date.year = 2016;

  memset(i2c_bytes, 0xFF, EEPROM_SIZE);
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
    i2c_bytes[addr_t+n_bytes] = (NULL == data) ? 0xFF : data[n_bytes];
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

  //  printf("ee24xx_read_bytes addr:%x num:%d\n", addr, num_bytes);
  for (n_bytes=0; n_bytes<num_bytes; n_bytes++) {
    if ((addr_t+n_bytes) >= EEPROM_SIZE) {
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
timerDateSet(date_t d)
{
  i2c_date = d;
}

#define timerDateGet(d)	d = i2c_date

void
timerTimeSet(s_time_t _t)
{
  i2c_time = _t;
}

#define timerTimeGet(t) t = i2c_time

//******************************************************************
//Function to get RTC date & time in FAT32 format
//  Return format : Year[31:25], Month[24:21], Date[20:16]
//                  Hour[15:11], Min[10:5], Sec[4:0]
//******************************************************************  
uint32_t
get_fattime (void)
{
  date_t   date;
  s_time_t   time;
  uint32_t dtFat;

  /* Process date */
  timerDateGet(date);
  dtFat = date.year;
  dtFat <<= 4;
  dtFat |= date.month;
  dtFat <<= 5;
  dtFat |= date.day;

  /* Process time */
  timerTimeGet(time);
  dtFat <<= 5;
  dtFat |= time.hour;
  dtFat <<= 6;
  dtFat |= time.min;
  /* FAT32 fromat accepts dates with 2sec resolution
     (e.g. value 5 => 10sec) */
  dtFat <<= 5;
  dtFat |= time.sec>>1;

  return dtFat;
}
