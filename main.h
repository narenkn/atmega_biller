#ifndef MAIN_H
#define MAIN_H

void main_init(void);

#define BUZZER_ON  PORTA |= 0x80
#define BUZZER_OFF PORTA &= ~0x80

//  Return format : Year[31:25], Month[24:21], Date[20:16]
//                  Hour[15:11], Min[10:5], Sec[4:0]
#define  FAT_SEC_OFFSET            0
#define  FAT_SEC_MASK           0x1F
#define  FAT_MIN_OFFSET            5
#define  FAT_MIN_MASK           0x3F
#define  FAT_HOUR_OFFSET          11
#define  FAT_HOUR_MASK          0x1F
#define  FAT_DATE_OFFSET          16
#define  FAT_DATE_MASK          0x1F
#define  FAT_MONTH_OFFSET         21
#define  FAT_MONTH_MASK          0xF
#define  FAT_YEAR_OFFSET          25
#define  FAT_YEAR_MASK          0x7F

extern volatile uint16_t timer2_beats;

typedef struct {
  uint8_t day;
  uint8_t month;
  uint16_t year;
} date_t;
typedef struct {
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
} s_time_t;

uint8_t  validDate(date_t date);
void nextDate(date_t *date);

void eeprom_setting2ram(void);

extern volatile uint8_t eeprom_setting0, eeprom_setting1;
#define EEPROM_SETTING_BUZZER      (1<<0)
#define EEPROM_SETTING0_ON(X)  eeprom_setting0 |= EEPROM_SETTING_ ## X
#define EEPROM_SETTING0_OFF(X) eeprom_setting0 &= ~EEPROM_SETTING_ ## X
#define EEPROM_SETTING0(X)  (eeprom_setting0 & EEPROM_SETTING_ ## X)

#define soft_reset()        \
  do {			    \
    wdt_enable(WDTO_15MS);  \
    for(;;) { }		    \
  } while(0)

#define WDT_RESET_WAKEUP (MCUCSR & _BV(WDRF))

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

#endif
