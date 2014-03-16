//**************************************************************
//******** FUNCTIONS FOR SERIAL COMMUNICATION USING UART *******
//**************************************************************
//Controller		: ATmega32 (Clock: 8 Mhz-internal)
//Compiler			: AVR-GCC (winAVR with AVRStudio-4)
//Project Version	: DL_1.0
//Author			: CC Dharmani, Chennai (India)
//			  		  www.dharmanitech.com
//Date				: 10 May 2011
//**************************************************************

#ifndef UART_H
#define UART_H

#define CHAR 0
#define INT  1
#define LONG 2

#define TX_NEWLINE {transmitByte(0x0d); transmitByte(0x0a);}

void uart0_init(void);
uint8_t receiveByte(void);
void transmitByte(uint8_t);
void transmitString_F(uint8_t *);
void transmitString(uint8_t *);
void transmitHex( uint8_t dataType, unsigned long data );


#endif
