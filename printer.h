#ifndef PRINTER_H
#define PRINTER_H

#define PRINTER_PAPER_STATUS        0x1B76
#define PRINTER_TOGGLE_ONOFF        0x1B3D
#define PRINTER_PRINT_FEED          0x1B4A
#define PRINTER_EMPHASIZE_ONOFF     0x1B45
#define PRINTER_INITIALIZE          0x1B40 /* -n */
#define PRINTER_UNDERLINE_ONOFF     0x1B2D
#define PRINTER_SLEEP_TIME          0x1B38 /* n1+n2*256 */
#define PRINTER_WAKE                0xFF
#define PRINTER_PRINT_TEST_PAGE     0x1254
#define PRINTER_PAPER_CUT           0x1B69
#define PRINTER_PAPER_PARTIAL_CUT   0x1B6D

#define PRINTER_PRINT(c) \
  uart_putc(c)

#define PRINTER_COMMAND(C) \
  uart_putc(C>>8); uart_putc(C)

#define PRINTER_CMD_ARG(N) \
  uart_putc(N)

#define PRINTER_ONLINE     \
  PRINTER_COMMAND(PRINTER_TOGGLE_ONOFF); \
  PRINTER_CMD_ARG(1)

void printer_init(void);
void printer_prn_date(uint8_t *ymd);
void printer_prn_uint16(uint16_t);
void printer_prn_str(uint8_t *str);

#endif
