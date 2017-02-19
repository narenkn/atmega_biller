#ifndef I2C_H
#define I2C_H

#define	 START			0x08
#define  REPEAT_START		0x10
#define  MT_SLA_ACK		0x18
#define  MT_SLA_NACK		0x20
#define  MT_DATA_ACK		0x28
#define  MT_DATA_NACK		0x30
#define  MR_SLA_ACK		0x40
#define  MR_SLA_NACK		0x48
#define  MR_DATA_ACK		0x50
#define  MR_DATA_NACK		0x58
#define  ARB_LOST		0x38

#define  ERROR_CODE		0x7e

#define TIMER_CTRL_WRITE        0xD0
#define TIMER_CTRL_READ         0xD1

#define TIMER_ADDR_SEC             0
#define TIMER_ADDR_MIN             1
#define TIMER_ADDR_HOUR            2
#define TIMER_ADDR_DATE            4
#define TIMER_ADDR_MONTH           5
#define TIMER_ADDR_YEAR            6

// External crystal frequency
#define RTC_F           32768

// Define delay
#define RTC_PERIOD_MS   1000

extern volatile uint8_t rtc_sec;
extern volatile uint8_t rtc_min;
extern volatile uint8_t rtc_hour;
extern volatile uint8_t rtc_date;
extern volatile uint8_t rtc_month;
extern volatile uint16_t rtc_year;

#define timerDateSet(date)			\
  rtc_year = date.year;				\
  rtc_month = date.month%12;			\
  rtc_date = date.day%31

#define timerDateGet(d)				\
  d.day = rtc_date, d.month = rtc_month, d.year = rtc_year

#define timerTimeSet(time)			\
  rtc_hour = time.hour;				\
  rtc_min = time.min;				\
  rtc_sec = time.sec

#define timerTimeGet(t)					\
  t.hour = rtc_hour, t.min = rtc_min, t.sec = rtc_sec

void     i2c_init(void);
uint16_t ee24xx_read_bytes(uint16_t eeaddr, uint8_t *buf, uint16_t len);
uint16_t ee24xx_write_bytes(uint16_t eeaddr, uint8_t *buf, uint16_t len);

/*
 * Compatibility defines.  This should work on ATmega8, ATmega16,
 * ATmega163, ATmega323 and ATmega128 (IOW: on all devices that
 * provide a builtin TWI interface).
 *
 * On the 128, it defaults to USART 1.
 */
#ifndef UCSRB
# ifdef UCSR1A		/* ATmega128 */
#  define UCSRA UCSR1A
#  define UCSRB UCSR1B
#  define UBRR UBRR1L
#  define UDR UDR1
# else /* ATmega8 */
#  define UCSRA USR
#  define UCSRB UCR
# endif
#endif
#ifndef UBRR
#  define UBRR UBRRL
#endif

/*
 * Note [3]
 * TWI address for 24Cxx EEPROM:
 *
 * 1 0 1 0 E2 E1 E0 R/~W	24C01/24C02
 * 1 0 1 0 E2 E1 A8 R/~W	24C04
 * 1 0 1 0 E2 A9 A8 R/~W	24C08
 * 1 0 1 0 A10 A9 A8 R/~W	24C16
 */
#define TWI_SLA_24CXX	0xa0	/* E2 E1 E0 = 0 0 0 */

/*
 * Note [3a]
 * Device word address length for 24Cxx EEPROM
 * Larger EEPROM devices (from 24C32) have 16-bit address
 * Define or undefine according to the used device
 */
#define WORD_ADDRESS_16BIT

/*
 * Maximal number of iterations to wait for a device to respond for a
 * selection.  Should be large enough to allow for a pending write to
 * complete, but low enough to properly abort an infinite loop in case
 * a slave is broken or not present at all.  With 100 kHz TWI clock,
 * transfering the start condition and SLA+R/W packet takes about 10
 * µs.  The longest write period is supposed to not exceed ~ 10 ms.
 * Thus, normal operation should not require more than 100 iterations
 * to get the device to respond to a selection.
 */
#define MAX_ITER	200

/*
 * Number of bytes that can be written in a row, see comments for
 * ee24xx_write_page() below.  Some vendor's devices would accept 16,
 * but 8 seems to be the lowest common denominator.
 *
 * Note that the page size must be a power of two, this simplifies the
 * page boundary calculations below.
 */
#define EEPROM_PAGE_SIZE 128

/*
 * Saved TWI status register, for error messages only.  We need to
 * save it in a variable, since the datasheet only guarantees the TWSR
 * register to have valid contents while the TWINT bit in TWCR is set.
 */
uint8_t twst;

/*
 * Number of EEPROM devices connected in the system
 * Can be 4 or 2 or 1
 */
#define EEPROM_MAX_DEVICES             1
#define EEPROM_MAX_DEVICES_LOGN2       0
#define EEPROM_ADDR_SHIFT              2

/* */
#define EEPROM_MAX_ADDRESS     ((uint16_t)((0x0FFF<<EEPROM_MAX_DEVICES_LOGN2)|0xF)&0xFFFF)

#if ! NVFLASH_EN

#define bill_read_bytes    ee24xx_read_bytes
#define bill_write_bytes   ee24xx_write_bytes

#endif

uint32_t get_fattime(void);

#endif
