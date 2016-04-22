#ifndef UART_H
#define UART_H

typedef enum { UART0_NONE=0, UART0_WEIGHMC, UART0_PC, UART0_PC_CMD } uart0_func_t;

#define PCPASS_SIZE (((SERIAL_NO_MAX-2)/3)*4)

void    uartInit(void);
void    uartSelect(uint8_t uid);
uint8_t uartReceiveByte(void);
void    uartTransmitByte(uint8_t);

extern uint32_t uartWeight;
extern uint8_t  uartDecimalPlace;
extern uint8_t uart0_func;
extern uint8_t pcPassword[];
extern uint8_t pcPassIdx;

#endif
