#ifndef UART_H
#define UART_H

enum { UART0_WEIGHMC=1, UART0_PC=2 } uart0_func_t;

void    uartInit(void);
void    uartSelect(uint8_t uid);
uint8_t uartReceiveByte(void);
void    uartTransmitByte(uint8_t);
void    uartTransmitString_P(uint8_t *);
void    uartTransmitString(uint8_t *);
void    uartTransmitHex( uint8_t dataType, unsigned long data );
inline void uart0_sel_func(uint8_t func);

#endif
