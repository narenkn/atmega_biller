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
uint8_t  validDate(uint8_t day, uint8_t month, uint8_t year);

#endif
