#ifndef RTC_H
#define RTC_H

#define		SECONDS			rtc_register[0]
#define		MINUTES			rtc_register[1]
#define		HOURS			rtc_register[2]
#define		DAY    			rtc_register[3]
#define		DATE   			rtc_register[4]
#define		MONTH  			rtc_register[5]
#define		YEAR   			rtc_register[6]

extern uint8_t time[9]; 	//xx:xx:xx;
extern uint8_t date[11];	//xx/xx/xxxx;
extern uint8_t day;
extern unsigned int dateFAT, timeFAT;
extern uint8_t rtc_register[7];

uint8_t RTC_setStartAddress(void);
uint8_t RTC_read(void);
uint8_t RTC_write(void);
uint8_t RTC_getTime(void);
uint8_t RTC_getDate(void);
void RTC_updateRegisters(void);
uint8_t RTC_writeTime(void);
uint8_t RTC_writeDate(void);
uint32_t get_fattime(void);

#endif
