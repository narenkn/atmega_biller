#ifndef UART_H
#define UART_H

#define UART0_BAUD   115200UL
#define UART1_BAUD     9600UL

void    uartInit(void);
/* Printer */
uint8_t uart0ReceiveByte(void);
void    uart0TransmitByte(uint8_t);
/* Weighing machine */
#if 0
uint8_t uart1ReceiveByte(void);
#endif
void    uart1TransmitByte(uint8_t);
void    uart1TransmitBit(void);

extern volatile uint32_t uartWeight;
extern volatile uint8_t  uartDecimalPlace;

#endif
