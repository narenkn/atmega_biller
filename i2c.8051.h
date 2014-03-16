#ifndef I2C_H
#define I2C_H

void    i2cInit(void);
void    eepromWriteBytes(uint16_t addr, uint8_t *val, uint8_t n_bytes);
void    eepromReadBytes(uint16_t addr, uint8_t *val, uint8_t n_bytes);
void    ds1307Write(uint8_t addr, uint8_t data);
uint8_t ds1307Read(uint8_t addr);

#define ACK      0
#define NACK     1

#define SCL      P3_4
#define SDA      P3_5

/* define it to K-bits */
#define EP24C                      2
#define EEPROM_SIZE       (EP24C<<7)

#define EEPROM_CTRL_WRITE 0xA0
#define EEPROM_CTRL_READ  0xA1
#define TIMER_CTRL_WRITE  0xD0
#define TIMER_CTRL_READ   0xD1

#define TIMER_ADDR_MIN    1
#define TIMER_ADDR_HOUR   2
#define TIMER_ADDR_DATE   4
#define TIMER_ADDR_MONTH  5
#define TIMER_ADDR_YEAR   6

#define NOP2 \
  __asm	     \
  NOP	     \
  NOP	     \
  __endasm

#define I2C_START_BIT	 \
  SDA = 1;		 \
  SCL = 1;		 \
  SDA = 0;		 \
  SCL = 0

#define I2C_STOP_BIT	 \
  SDA = 0;		 \
  SCL = 1;		 \
  SDA = 1

#define I2C_GET_ACK(a)	   \
  SDA = 1;		   \
  SCL = 1;		   \
  a = SDA;		   \
  SCL = 0


#define I2C_SEND_ACK(a)	   \
  SDA = a;		   \
  SCL = 1;		   \
  SCL = 0

#define I2C_SEND_BYTE(b)    \
  for (ui3=8; ui3>0;) {	    \
    ui3--;		    \
    CY = (b >> ui3) & 0x1;  \
    SDA = CY;		    \
    SCL=1;		    \
    SCL=0;		    \
  }

#define I2C_GET_BYTE(b)     \
  SDA = 1;		    \
  b = 0;		    \
  for (ui3=0; ui3<8;ui3++) {	    \
    SCL=1;		    \
    b <<= 1;		    \
    b |= SDA;		    \
    SCL=0;		    \
  }

#define timerDateSet(year, month, date) \
  DISABLE_ALL_INTERRUPTS;		\
  ds1307Write(0x04, date);			\
  ds1307Write(0x05, month);		\
  ds1307Write(0x06, year);			\
  ENABLE_ALL_INTERRUPTS

#define timerDateGet(ymd)			\
  DISABLE_ALL_INTERRUPTS;			\
  ymd[2] = ds1307Read((uint8_t)TIMER_ADDR_DATE);	\
  ymd[1] = ds1307Read((uint8_t)TIMER_ADDR_MONTH);	\
  ymd[0] = ds1307Read((uint8_t)TIMER_ADDR_YEAR);	\
  ENABLE_ALL_INTERRUPTS

#define timerTimeSet(hour, min)			\
  DISABLE_ALL_INTERRUPTS;			\
  ds1307Write(0x01, min);				\
  ds1307Write(0x02, hour);				\
  ds1307Write(0x00, 0);				\
  ENABLE_ALL_INTERRUPTS

#define timerTimeGet(hm)			\
  DISABLE_ALL_INTERRUPTS;			\
  hm[1] = ds1307Read((uint8_t)TIMER_ADDR_MIN);	\
  hm[0] = ds1307Read((uint8_t)TIMER_ADDR_HOUR);	\
  ENABLE_ALL_INTERRUPTS

#define timerRamStore1(addr, val)		\
  DISABLE_ALL_INTERRUPTS;			\
  ds1307Write((uint8_t)addr, (uint8_t)val);	\
  ENABLE_ALL_INTERRUPTS

#define timerRamRestore1(addr, val)	\
  DISABLE_ALL_INTERRUPTS;		\
  val = ds1307Read((uint8_t)addr);	\
  ENABLE_ALL_INTERRUPTS

#define timerRamStore2(addr, val)		\
  DISABLE_ALL_INTERRUPTS;			\
  ds1307Write((uint8_t)addr, (uint8_t)val);	\
  ds1307Write((uint8_t)(addr+1), (uint8_t)(val>>8));	\
  ENABLE_ALL_INTERRUPTS

#define timerRamRestore2(addr, val)	\
  DISABLE_ALL_INTERRUPTS;		\
  val = ds1307Read((uint8_t)(addr+1));	\
  val <<= 8;				\
  val |= ds1307Read((uint8_t)addr);	\
  ENABLE_ALL_INTERRUPTS

#define PREV_MONTH(M)       \
  ((M==1) ? 12 : (M-1))

#define INCR_MONTH(M)       \
  ((M==12) ? 1 : (M+1))

#endif
