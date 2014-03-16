#include <stdint.h>
#include <p89v51rd2.h>

#include "i2c.h"

#ifdef UNIT_TEST
#error "This file should not be included in Unit tests"
#endif

void
i2cInit(void)
{
  SDA = 1;
  SCL = 1;
}

void
eepromWriteBytes(uint16_t addr, uint8_t *val, uint8_t n_bytes)
{
  uint8_t ui2, ui3;

  DISABLE_ALL_INTERRUPTS;

  for (; n_bytes>0; n_bytes--, val++, addr++) {
    I2C_START_BIT;
    I2C_SEND_BYTE(EEPROM_CTRL_WRITE);   
    I2C_GET_ACK(CY);

#if EP24C > 2
    ui2 = addr>>8;
    I2C_SEND_BYTE(ui2);
    I2C_GET_ACK(CY);
#endif

    ui2 = (uint8_t)addr;
    I2C_SEND_BYTE(ui2);
    I2C_GET_ACK(CY);

    I2C_SEND_BYTE(val[0]);
    I2C_GET_ACK(CY);
    I2C_STOP_BIT;

    delayms(10);
  }

  ENABLE_ALL_INTERRUPTS;
}

void
eepromReadBytes(uint16_t addr, uint8_t *val, uint8_t n_bytes)
{
  uint8_t ui2, ui3;

  DISABLE_ALL_INTERRUPTS;

  for (; n_bytes>0; n_bytes--, val++, addr++) {
    I2C_START_BIT;

    I2C_SEND_BYTE(EEPROM_CTRL_WRITE);   
    I2C_GET_ACK(CY);

#if EP24C > 2
    ui2 = addr>>8;
    I2C_SEND_BYTE(ui2);
    I2C_GET_ACK(CY);
#endif

    ui2 = (uint8_t)addr;
    I2C_SEND_BYTE(ui2);
    I2C_GET_ACK(CY);

    I2C_START_BIT;
    I2C_SEND_BYTE(EEPROM_CTRL_READ);   
    I2C_GET_ACK(CY);

    I2C_GET_BYTE(ui2);
    I2C_SEND_ACK(1);
    val[0] = ui2;

    I2C_STOP_BIT;
  }

  ENABLE_ALL_INTERRUPTS;
}
    
void
ds1307Write(uint8_t addr, uint8_t data)
{
  uint8_t ui3;

  I2C_START_BIT;

  do {
    I2C_SEND_BYTE(TIMER_CTRL_WRITE);
    I2C_GET_ACK(CY);
  } while (NACK == CY);

  do {
    I2C_SEND_BYTE(addr);
    I2C_GET_ACK(CY);
  } while (NACK == CY);

  do {
    I2C_SEND_BYTE(data);
    I2C_GET_ACK(CY);
  } while (NACK == CY);

  CY=0;
  I2C_STOP_BIT;
}

uint8_t
ds1307Read(uint8_t addr)
{
  uint8_t ui3, data;

  I2C_START_BIT;

  do {
    I2C_SEND_BYTE(TIMER_CTRL_WRITE);
    I2C_GET_ACK(CY);
  } while (NACK == CY);

  do {
    I2C_SEND_BYTE(addr);
    I2C_GET_ACK(CY);
  } while (NACK == CY);

  I2C_START_BIT;

  do {
    I2C_SEND_BYTE(TIMER_CTRL_READ);
    I2C_GET_ACK(CY);
  } while (NACK == CY);

  I2C_GET_BYTE(data);
  I2C_SEND_ACK(1);
  CY=0;
  I2C_STOP_BIT;

  return(data);
}
