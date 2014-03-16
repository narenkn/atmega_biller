//*******************************************************
//  *******  RTC_ROUTINES HEADER FILE ******** 		
//*******************************************************
//Controller		: ATmega32 (Clock: 8 Mhz-internal)
//Compiler			: AVR-GCC (winAVR with AVRStudio-4)
//Project Version	: DL_1.0
//Author			: CC Dharmani, Chennai (India)
//			  		  www.dharmanitech.com
//Date				: 10 May 2011
//*******************************************************

#ifndef RTC_H
#define RTC_H

#define		SECONDS			rtc_register[0]
#define		MINUTES			rtc_register[1]
#define		HOURS			rtc_register[2]
#define		DAY    			rtc_register[3]
#define		DATE   			rtc_register[4]
#define		MONTH  			rtc_register[5]
#define		YEAR   			rtc_register[6]

uint8_t time[9]; 	//xx:xx:xx;
uint8_t date[11];	//xx/xx/xxxx;
uint8_t day;
unsigned int dateFAT, timeFAT;
uint8_t rtc_register[7];

uint8_t RTC_setStartAddress(void);
uint8_t RTC_read(void);
uint8_t RTC_write(void);
uint8_t RTC_getTime(void);
uint8_t RTC_getDate(void);
uint8_t RTC_displayTime(void);
uint8_t RTC_displayDate(void);
void RTC_displayDay(void);
void RTC_updateRegisters(void);
uint8_t RTC_writeTime(void);
uint8_t RTC_writeDate(void);
uint8_t RTC_updateTime(void);
uint8_t RTC_updateDate(void);
uint8_t getDateTime_FAT(void);


#endif
	
