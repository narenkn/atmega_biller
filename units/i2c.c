#include "i2c.h"

#ifndef UNIT_TEST
#error  "This file should not be used in non-Unit test mode"
#endif

uint8_t i2c_ymd[3], i2c_hm[2];

#define NIBBLE_PACK(A, B) ((A<<4)|B)
#define EPROM_SIZE ((1<<12)<<2)
uint8_t i2c_bytes[EEPROM_SIZE];

void
i2c_init(void)
{
  i2c_ymd[0] = NIBBLE_PACK(0, 1);
  i2c_ymd[1] = NIBBLE_PACK(0, 1);
  i2c_ymd[2] = NIBBLE_PACK(1, 4);

  uint16_t ui1;
  for (ui1=0; ui1<EEPROM_SIZE; ui1++)
    i2c_bytes[ui1] = 0xBD;
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
i2c_sendAddress(uint8_t)
{
  assert(0);
  return 0;
}

uint8_t
i2c_sendData(uint8_t)
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
  return 0;
}

#define ee24xx_write_page ee24xx_write_bytes

uint16_t
ee24xx_write_bytes(uint16_t addr, uint8_t *data, uint16_t num_bytes)
{
  uint16_t n_bytes = num_bytes;
  assert (addr < EEPROM_SIZE);
  if (addr < EEPROM_SIZE) {
    for (; n_bytes>0; n_bytes--) {
      i2c_bytes[addr] = data[0];
      addr++, data++;
    }
  }

  return num_bytes;
}

uint16_t
ee24xx_read_bytes(uint16_t addr, uint8_t *data, uint16_t num_bytes)
{
  uint16_t n_bytes = num_bytes;

  assert (addr < EEPROM_SIZE);
  for (; n_bytes>0; n_bytes--) {
    data[0] = i2c_bytes[addr];
    addr++, data++;
  }

  return num_bytes;
}

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
  i2c_hm[0] = min;
  i2c_hm[1] = hour;
}

void
timerTimeGet(uint8_t *hm)
{
  hm[0] = i2c_hm[0];
  hm[1] = i2c_hm[1];
}
