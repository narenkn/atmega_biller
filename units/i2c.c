#include "i2c.h"

date_t i2c_date;
s_time_t i2c_time;

void
i2c_init(void)
{
  i2c_date.day = 1;
  i2c_date.month = 10;
  i2c_date.year = 2016;
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
  dtFat = (date.year-2000);
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
