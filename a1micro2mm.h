#ifndef PRINTER_H
#define PRINTER_H

#define PRINTER_MAX_CHARS_ON_LINE   42

#define PRINTER_PRINT(c)			\
  uartTransmitByte(c)

#define ASCII_ESCAPE        0x1B

#define PRINTER_PAPER_STATUS			\
  PRINTER_PRINT(ASCII_ESCAPE); PRINTER_PRINT(0x76)
#define PRINTER_TOGGLE_ONOFF			\
  PRINTER_PRINT(ASCII_ESCAPE); PRINTER_PRINT(0x3D)
#define PRINTER_PRINT_FEED			\
  PRINTER_PRINT(ASCII_ESCAPE); PRINTER_PRINT(0x4A)
#define PRINTER_EMPHASIZE_ONOFF			\
  PRINTER_PRINT(ASCII_ESCAPE); PRINTER_PRINT(0x45)
#define PRINTER_INITIALIZE			\
  PRINTER_PRINT(ASCII_ESCAPE); PRINTER_PRINT('@') /* -n */
#define PRINTER_UNDERLINE_ONOFF			\
  PRINTER_PRINT(ASCII_ESCAPE); PRINTER_PRINT(0x2D)
#define PRINTER_WAKE				\
  PRINTER_PRINT(0xFF)
#define PRINTER_PRINT_TEST_PAGE			\
  PRINTER_PRINT(0x12); PRINTER_PRINT(0x54)
#define PRINTER_PAPER_CUT			\
  PRINTER_PRINT(ASCII_ESCAPE); PRINTER_PRINT(0x69)
#define PRINTER_PAPER_PARTIAL_CUT		\
  PRINTER_PRINT(ASCII_ESCAPE); PRINTER_PRINT(0x6D)
#define PRINTER_SLEEP_SET(N)			\
  PRINTER_PRINT(ASCII_ESCAPE); PRINTER_PRINT('8'); PRINTER_PRINT(N)

#define PRINTER_ONLINE				\
  PRINTER_TOGGLE_ONOFF;				\
  PRINTER_CMD_ARG(1)

#define PRINTER_OFFLINE				\
  PRINTER_TOGGLE_ONOFF;				\
  PRINTER_CMD_ARG(0)

#define PRINTER_COMMAND                  uartTransmitByte
#define PRN_CMD_NL                       0xA
#define PRN_CMD_FF                       0xC
#define PRN_CMD_HT                       0x9
#define PRN_MODE_REVERSE                 (1<<1)
#define PRN_MODE_EMPHASIZE               (1<<3)
#define PRN_MODE_DOUBLE_HEIGHT           (1<<4)
#define PRN_MODE_DOUBLE_WIDTH            (1<<5)

#define PRINTER_FEED_DOTS(n)			\
  uartTransmitByte(ASCII_ESCAPE);		\
  uartTransmitByte('J');			\
  uartTransmitByte(n)

#define PRINTER_FEED_LINES(n)			\
  uartTransmitByte(ASCII_ESCAPE);		\
  uartTransmitByte('d');			\
  uartTransmitByte(n)

#define PRINTER_FONT_ENLARGE(N)			\
  uartTransmitByte(0x1D);			\
  uartTransmitByte('!');			\
  uartTransmitByte(N)

#define PRINTER_BOLD(N)				\
  uartTransmitByte(ASCII_ESCAPE);		\
  uartTransmitByte('!');			\
  uartTransmitByte(N)

#define PRINTER_UNDERLINE(N)			\
  uartTransmitByte(ASCII_ESCAPE);		\
  uartTransmitByte('-');			\
  uartTransmitByte(N)

#define PRINTER_USERCHAR_ENDIS(N)		\
  uartTransmitByte(ASCII_ESCAPE);		\
  uartTransmitByte('%');			\
  uartTransmitByte(N)

void printerInit(void);
void printerDefineUserChar(uint8_t idx);
void printerStatus(void);

#endif
