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

#define TX_NEWLINE	   \
  uart_transmitByte(0x0d); \
  uart_transmitByte(0x0a)

void    uart_init(void);
void    uart_select(uint8_t uid);
uint8_t uart_receiveByte(void);
void    uart_transmitByte(uint8_t);
void    uart_transmitString_F(uint8_t *);
void    uart_transmitString(uint8_t *);
void    uart_transmitHex( uint8_t dataType, unsigned long data );

#endif
