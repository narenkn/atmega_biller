#ifndef UART_H
#define UART_H

#define CHAR 0
#define INT  1
#define LONG 2

#define UART_TX_NEWLINE	   \
  uart_transmitByte('\r'); \
  uart_transmitByte('\n')

void    uartInit(void);
void    uartSelect(uint8_t uid);
uint8_t uartReceiveByte(void);
void    uartTransmitByte(uint8_t);
void    uartTransmitString_P(uint8_t *);
void    uartTransmitString(uint8_t *);
void    uartTransmitHex( uint8_t dataType, unsigned long data );

#endif
