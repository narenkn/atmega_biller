#ifndef MAIN_H
#define MAIN_H

void main_init(void);

#define BUZZER_ON  PORTD |= 0x80
#define BUZZER_OFF PORTD &= ~0x80

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

extern volatile uint8_t timer2_msb;
uint8_t  validDate(uint8_t day, uint8_t month, uint8_t year);
extern volatile uint16_t timer2_sleep_delay;

void eeprom_setting2ram();

extern volatile uint8_t eeprom_setting0, eeprom_setting1;
#define EEPROM_SETTING_BUZZER      (1<<0)
#define EEPROM_SETTING0_ON(X)  eeprom_setting0 |= EEPROM_SETTING_ ## X
#define EEPROM_SETTING0_OFF(X) eeprom_setting0 &= ~EEPROM_SETTING_ ## X
#define EEPROM_SETTING0(X)  (eeprom_setting0 & EEPROM_SETTING_ ## X)

#endif
