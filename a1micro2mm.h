#ifndef PRINTER_H
#define PRINTER_H

#define PRINTER_MAX_CHARS_ON_LINE   32

#define PRINTER_PRINT(c)			\
  uart0TransmitByte(c)

#define PRINTER_PRINT_DEC(d)			\

#define ASCII_PRINTER_ESC        0x1B

#define PRINTER_PAPER_STATUS			\
  PRINTER_PRINT(ASCII_PRINTER_ESC); PRINTER_PRINT('v')
#define PRINTER_TOGGLE_ONOFF			\
  PRINTER_PRINT(ASCII_PRINTER_ESC); PRINTER_PRINT(0x3D)
#define PRINTER_PRINT_FEED			\
  PRINTER_PRINT(ASCII_PRINTER_ESC); PRINTER_PRINT(0x4A)
#define PRINTER_EMPHASIZE_ONOFF			\
  PRINTER_PRINT(ASCII_PRINTER_ESC); PRINTER_PRINT(0x45)
#define PRINTER_INITIALIZE			\
  PRINTER_PRINT(ASCII_PRINTER_ESC); PRINTER_PRINT('@') /* -n */
#define PRINTER_UNDERLINE_ONOFF			\
  PRINTER_PRINT(ASCII_PRINTER_ESC); PRINTER_PRINT(0x2D)
#define PRINTER_WAKE				\
  PRINTER_PRINT(0xFF)
#define PRINTER_PRINT_TEST_PAGE			\
  PRINTER_PRINT(0x12); PRINTER_PRINT(0x54)
#define PRINTER_PAPER_CUT			\
  PRINTER_PRINT(ASCII_PRINTER_ESC); PRINTER_PRINT(0x69)
#define PRINTER_PAPER_PARTIAL_CUT		\
  PRINTER_PRINT(ASCII_PRINTER_ESC); PRINTER_PRINT(0x6D)
#define PRINTER_SLEEP_SET(N)			\
  PRINTER_PRINT(ASCII_PRINTER_ESC); PRINTER_PRINT('8'); PRINTER_PRINT(N)
#define PRINTER_JLEFT   0
#define PRINTER_JCENTER 1
#define PRINTER_JRIGHT  2
#define PRINTER_JUSTIFY(N)			\
  PRINTER_PRINT(ASCII_PRINTER_ESC); PRINTER_PRINT('a'); \
  PRINTER_PRINT(N)


#define PRINTER_ONLINE				\
  PRINTER_TOGGLE_ONOFF;				\
  PRINTER_CMD_ARG(1)

#define PRINTER_OFFLINE				\
  PRINTER_TOGGLE_ONOFF;				\
  PRINTER_CMD_ARG(0)

#define PRINTER_COMMAND                  uart0TransmitByte
#define PRN_CMD_NL                       0xA
#define PRN_CMD_FF                       0xC
#define PRN_CMD_HT                       0x9
#define PRN_MODE_REVERSE                 (1<<1)
#define PRN_MODE_EMPHASIZE               (1<<3)
#define PRN_MODE_DOUBLE_HEIGHT           (1<<4)
#define PRN_MODE_DOUBLE_WIDTH            (1<<5)

#define PRINTER_STATUS_ONLINE            (1<<0)
#define PRINTER_STATUS_PAPEROUT          (1<<2)
#define PRINTER_STATUS_VHIGH             (1<<3)
#define PRINTER_STATUS_THIGH             (1<<6)

void PRINTER_FEED_DOTS(uint8_t n);
void PRINTER_FEED_LINES(uint8_t n);
void PRINTER_FONT_ENLARGE(uint8_t N);
void PRINTER_BOLD(uint8_t N);
void PRINTER_UNDERLINE(uint8_t N);
void PRINTER_USERCHAR_ENDIS(uint8_t N);
void PRINTER_PSTR(const char *P);

#define PRINTER_PRINT_D menuPrnD
#define PRINTER_PRINT_F menuPrnF

#define PRINTER_SPRINTF(STR, FMT, ...)	do {		\
    uint8_t ui8_1t, ui8_2t;				\
    ui8_2t = sprintf_P((char *)STR, FMT, __VA_ARGS__);	\
    for (ui8_1t=0; ui8_1t<ui8_2t; ui8_1t++) {		\
      PRINTER_PRINT(STR[ui8_1t]);			\
    }							\
  } while (0)

#define PRINTER_PRINTN(STR, N)				\
  for (uint8_t _ui8_1=0; _ui8_1<N; _ui8_1++) {		\
    PRINTER_PRINT(STR[_ui8_1]);				\
  }

void printerInit(void);
void printerDefineUserChar(uint8_t idx);
uint8_t printerStatus(void);

#endif
