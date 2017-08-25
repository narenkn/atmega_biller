/****************************************************************************
Title:    Access serial EEPROM 24C02 using I2C interace
Author:   Peter Fleury <pfleury@gmx.ch>
File:     $Id: test_i2cmaster.c,v 1.3 2015/09/16 09:29:24 peter Exp $
Software: AVR-GCC 4.x
Hardware: any AVR device can be used when using i2cmaster.S or any
          AVR device with hardware TWI interface when using twimaster.c

Description:
    This example shows how the I2C/TWI library i2cmaster.S or twimaster.c 
	can be used to access a serial eeprom.
 
#define Dev24C02  0xA2      // device address of EEPROM 24C02, see datasheet
//  unsigned char ret;
//  i2c_init();                                // init I2C interface
//  // write 0x75 to eeprom address 0x05 (Byte Write)
//  ret = i2c_start(Dev24C02+I2C_WRITE);       // set device address and write mode
//  if ( ret ) {
//    // failed to issue start condition, possibly no device found
//    i2c_stop();
//  } else {
//    // issuing start condition ok, device accessible
//    i2c_write(0x05);                       // write address = 5
//    i2c_write(0x75);                       // ret=0 -> Ok, ret=1 -> no ACK 
//    i2c_stop();                            // set stop conditon = release bus
//
//    // write ok, read value back from eeprom address 0x05, wait until 
//    //   the device is no longer busy from the previous write operation
//    i2c_start_wait(Dev24C02+I2C_WRITE);     // set device address and write mode
//    i2c_write(0x05);                        // write address = 5
//    i2c_rep_start(Dev24C02+I2C_READ);       // set device address and read mode
//    ret = i2c_readNak();                    // read one byte
//    i2c_stop();
//        
//    PORTB = ~ret;                           // output byte on the LED's
//
//    // write 0x70,0x71,072,073 to eeprom address 0x00..0x03 (Page Write),
//    //   wait until the device is no longer busy from the previous write operation
//    i2c_start_wait(Dev24C02+I2C_WRITE);     // set device address and write mode
//    i2c_write(0x00);                        // write start address = 0
//    i2c_write(0x70);                        // write data to address 0
//    i2c_write(0x71);                        //    "    "   "    "    1
//    i2c_write(0x72);                        //    "    "   "    "    2
//    i2c_write(0x74);                        //    "    "   "    "    3
//    i2c_stop();                             // set stop conditon = release bus
//    
//    // write ok, read value back from eeprom address 0..3 (Sequencial Read),
//    //   wait until the device is no longer busy from the previous write operation
//    i2c_start_wait(Dev24C02+I2C_WRITE);      // set device address and write mode
//    i2c_write(0x00);                         // write address = 0
//    i2c_rep_start(Dev24C02+I2C_READ);        // set device address and read mode
//    ret = i2c_readAck();                       // read one byte form address 0
//    ret = i2c_readAck();                       //  "    "    "    "     "    1
//    ret = i2c_readAck();                       //  "    "    "    "     "    2
//    ret = i2c_readNak();                       //  "    "    "    "     "    3
//    i2c_stop();                              // set stop condition = release bus
//    
//    PORTB = ~ret;                            // output byte on the LED's        
//  }
*****************************************************************************/

#include <avr/interrupt.h>
#include <avr/boot.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/crc16.h>
#include <stdint.h>

#include "i2cmaster.h"

/* globals */
uint16_t regulator_voltage, battery_voltage;
typedef void (*AppPtr_t)(void) __attribute__((noreturn));

/* Reprogram flash */
#define  BOOTLOADER_SIZE    (512)
#define  BOOTSTART ((1<<10)-BOOTLOADER_SIZE)
void bootload(void) BOOTLOADER_SECTION;
void
bootload()
{
  /* no more interrupts */
  cli();
  eeprom_busy_wait();

  uint8_t pageHasValidData;
  for (uint16_t page=0; ; page+=SPM_PAGESIZE) {
    /* */
    if (page >= BOOTSTART) break;

    // erase the page
    boot_page_erase(page);
    boot_spm_busy_wait();

    // Fill page with data from the buffer
    pageHasValidData = 0;
    for (uint16_t index=0, data; index<SPM_PAGESIZE; index += 2) {
      i2c_start_wait(I2C_WRITE);
      i2c_write(page>>8);
      i2c_write(page);
      i2c_rep_start(I2C_READ);
      data = i2c_readAck();
      data <<= 8;
      data |= i2c_read(((index+2)<SPM_PAGESIZE));
      boot_page_fill(page+index, data);
      if ((0 == pageHasValidData) && (0xFFFF != data)) {
	pageHasValidData = 1;
      }
    }
    if (0 == pageHasValidData) continue;

    // Write the data and wait for it to finish
    boot_page_write(page);
    boot_spm_busy_wait();
  }

  /* */
  ((AppPtr_t)0)();
}

#define WATCHDOG_DISABLE {			\
  MCUSR &= ~(1<<WDRF);				\
  WDTCR = (1<<WDCE) | (1<<WDE);			\
  WDTCR = 0x0;					\
  }
#define WATCHDOG_ENABLE {			\
  WDTCR = (1 << WDCE) | (1 << WDE);			\
  WDTCR = (1 << WDTIE) | (1 << WDP3) | (1 << WDP0);	\
  }

inline void
init(void)
{
  /* */
  cli();
  sleep_disable();
  WATCHDOG_DISABLE;

  i2c_init();

  // Set the ADC input to PB2/ADC1, left adjust result
  ADMUX |= (1 << ADLAR);
  // Set the prescaler to clock/128 & enable ADC
  ADCSRA |= (1 << ADEN);

  /* PWM setup for FAN */
  // Set Timer 0 prescaler to clock/1024.
  TCCR0B |= (1 << CS02) | (1 << CS00);
  // Set to Toggle mode
  TCCR0A |= (1 << COM0A1);

  /* interrupt from linux */
  PCMSK |= (1 << PCINT1);
  GIMSK |= (1 << PCIE);

  /* Sleep enable : MCUCR |= (1 << SE) | (1 << SM1); */
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();

  sei();
}

ISR(WDT_vect)
{
}
ISR(PCINT0_vect)
{
}

/* ADC = IN*1024/VREF ; 3.3 V => 0xFF; so each value is 3.3/0xFF
   IN = ADC * 33 / 1024 ; IN = 33 * adc / 256 ;
   each 1 unit is 0.0128 volts * 256 = 3.2768
 */
#define ADC_REG5V_PIN 0
#define ADC_BATT_PIN  3
uint16_t
adc_read_volt(uint8_t pin)
{
  ADMUX |= (1 << pin);

  for (uint8_t ui1=2; ui1; ui1--) {
    // Start the conversion
    ADCSRA |= (1 << ADSC);
    // Wait for it to finish - blocking
    while (ADCSRA & (1 << ADSC));
  }

  uint16_t ret = ADCH;
  ret *= 128;
  return ret;
}

#define I2C_ADDR_BASE 0xAB
#define I2C_ADDR_READ_BASE 0xAC
uint8_t
i2cSendData(uint8_t *d)
{
  uint8_t numSent = 0;
  uint8_t crc = 0;

  if ( i2c_start(I2C_ADDR_BASE) ) {
    i2c_stop();
    return 0;
  }

  for (; d[0]; d++, numSent++) {
    i2c_write(d[0]);
    crc = _crc_ibutton_update(crc, d[0]);
  }
  i2c_write(crc);
  i2c_stop();

  return numSent+1;
}

uint8_t
i2cReceiveData(uint8_t *d, uint8_t numData)
{
  uint8_t crc = 0;

  i2c_start_wait(I2C_ADDR_READ_BASE);
  i2c_write(numData);
  i2c_rep_start(I2C_ADDR_READ_BASE);

  for (uint8_t nd=numData; nd; d++, nd--) {
    d[0] = i2c_readAck();
    crc = _crc_ibutton_update(crc, d[0]);
  }
  crc ^= i2c_readNak();
  i2c_stop();

  return (0 == crc) ? numData : 0;
}

int
main(void)
{
  uint8_t num_wakeups;

  init();

  for (num_wakeups=0;;) {
    /* Enter powerdown mode */
    WATCHDOG_ENABLE;
    sleep_cpu();
    WATCHDOG_DISABLE;
    num_wakeups++;

    /* if pinchange interrupt, then ask what to do, else housekeep */
    if (GIFR & (1<<PCIF)) {
      bootload();
    } else if (num_wakeups & 0xF0) { /* once in 16 wakeups */
      /* Housekeeping */
      regulator_voltage = adc_read_volt(ADC_REG5V_PIN);
      battery_voltage = adc_read_volt(ADC_BATT_PIN);
      /* send data to host */
      
      /* send */
      num_wakeups = 0;
    }
  }

  for(;;);
}
