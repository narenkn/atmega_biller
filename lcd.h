#ifndef LCD__H
#define LCD__H

#if LCD2004
#define LCD_MAX_ROW            4
#define LCD_MAX_COL           20
#else
#define LCD_MAX_ROW            2
#define LCD_MAX_COL           16
#endif

#if 8 == LCD_DPORT_SIZE
# define LCD_PORT(val)  PORTD &= ~0xF0; PORTD |= val & 0xF0; PORTA &= ~0xF; PORTA |= val & 0xF
#elif 4 == LCD_DPORT_SIZE
# define LCD_PORT(val)				\
  PORTD = (PORTD & ~0xF0) | ((val<<4) & 0xF0)
#endif
#define LCD_en_high  PORTB |= _BV(7)
#define LCD_en_low   PORTB &= ~_BV(7)
#define LCD_rs_high  PORTB |= _BV(5)
#define LCD_rs_low   PORTB &= ~_BV(5)
#define LCD_rw_high  PORTB |= _BV(6)
#define LCD_rw_low   PORTB &= ~_BV(6)
#define LCD_bl_on    PORTG |= _BV(2)
#define LCD_bl_off   PORTG &= ~_BV(2)
#define LCD_IS_ON   (PING & _BV(2))

#if UNIT_TEST

# define LCD_CMD_DISON_CURON_BLINKON assert(0)
# define LCD_CMD_CLRSCR        lcd_clrscr()
# define LCD_CMD_HOME          lcd_buf_p = (uint8_t *)lcd_buf
# define LCD_CMD_DEC_CUR
# define LCD_CMD_INC_CUR       assert(0)
# define LCD_CMD_DISON_CURON   assert(0)
# define LCD_CMD_CUR_10        0x80
# define LCD_CMD_CUR_20        0xC0
#if LCD2004
# define LCD_CMD_CUR_30        0x94
# define LCD_CMD_CUR_40        0xD4
#endif
# define LCD_CMD_2LINE_5x7     assert(0)
# define LCD_ACT_LINE2         assert(0)
# define LCD_cmd(CMD)					\
  do {							\
  if ((CMD >= 0x80) & (CMD <= 0x93)) {			\
    lcd_buf_p = ((uint8_t *)lcd_buf) + (CMD ^ 0x80);	\
    lcd_x = 0;						\
    lcd_y = CMD & 0xF;					\
  } else if ((CMD >= 0xC0) & (CMD <= 0xD3)) {		\
    lcd_buf_p = ((uint8_t *)lcd_buf) + (CMD ^ 0xC0);	\
    lcd_x = 1;						\
    lcd_y = CMD & 0xF;					\
  } else if ((CMD >= 0x94) & (CMD <= 0xA7)) {		\
    lcd_buf_p = ((uint8_t *)lcd_buf) + (CMD-0x94);	\
    lcd_x = 1;						\
    lcd_y = CMD & 0xF;					\
  } else if ((CMD >= 0xD4) & (CMD <= 0xE7)) {		\
    lcd_buf_p = ((uint8_t *)lcd_buf) + (CMD-0xD4);	\
    lcd_x = 1;						\
    lcd_y = CMD & 0xF;					\
  }							\
  } while (0)
# define LCD_idle_drive

# define LCD_refresh()          // FIXME: put debug info

# define KBD_GETCH_ALERT_N do { } while (0)

#else

# define LCD_CMD_DISON_CURON_BLINKON 0x0F /* LCD ON, Cursor ON, Cursor blinking ON */
# define LCD_CMD_CLRSCR        0x01   /* Clear screen */
# define LCD_CMD_HOME          0x02   /* Return home */
# define LCD_CMD_DEC_CUR       0x04   /* Decrement cursor */
# define LCD_CMD_INC_CUR       0x06   /* Increment cursor */
# define LCD_CMD_DISON_CURON   0x0E   /* Display ON ,Cursor ON */
# define LCD_CMD_CUR_10        0x80   /* Force cursor to the beginning of 1st line */
# define LCD_CMD_CUR_MOVL      0x10   /* Move cursor to left one char */
# define LCD_CMD_CUR_20        0xC0   /* Force cursor to the beginning of 2nd line */
#if LCD2004
# define LCD_CMD_CUR_30        0x94
# define LCD_CMD_CUR_40        0xD4
#endif
# define LCD_ACT_LINE2         0x3C   /* Activate second line */

# define KBD_GETCH_ALERT_N     KBD_GETCH

#if 8 == LCD_DPORT_SIZE

# define LCD_CMD_2LINE_5x7     0x38   /* Use 2 lines and 5×7 matrix */

# define LCD_cmd(var)   \
  LCD_rw_low;		\
  LCD_PORT(var);	\
  LCD_rs_low;		\
  _delay_us(50);	\
  LCD_en_high;		\
  _delay_us(50);	\
  LCD_en_low;		\
  LCD_rw_high;		\
  _delay_us(50)

# define LCD_idle_drive \
  LCD_PORT(0);		\
  LCD_rs_low;		\
  LCD_en_low;		\
  LCD_rw_high;		\
  _delay_us(100)

# define LCD_wrchar(var)\
  LCD_rw_low;		\
  LCD_PORT(var);	\
  LCD_rs_high;		\
  _delay_us(50);	\
  LCD_en_high;		\
  _delay_us(50);	\
  LCD_en_low;		\
  LCD_rw_high;		\
  _delay_us(50)

#elif 4 == LCD_DPORT_SIZE

# define LCD_CMD_2LINE_5x7     0x28   /* Use 2 lines and 5×7 matrix */

# define LCD_wrnib(var)	\
  LCD_rw_low;		\
  LCD_PORT(var);	\
  _delay_us(50);	\
  LCD_rs_high;		\
  _delay_us(50);	\
  LCD_en_high;		\
  _delay_us(50);	\
  LCD_en_low;		\
  LCD_rw_high;		\
  _delay_us(50)

# define LCD_wrchar(var)\
  LCD_rw_low;		\
  LCD_wrnib(var>>4);	\
  _delay_us(50);	\
  LCD_wrnib(var);	\
  LCD_rw_high;		\
  _delay_us(50)

# define LCD_cmd(var)   \
  LCD_rw_low;		\
  LCD_PORT(var>>4);	\
  _delay_us(50);	\
  LCD_rs_low;		\
  _delay_us(50);	\
  LCD_en_high;		\
  _delay_us(50);	\
  LCD_en_low;		\
  _delay_us(50);	\
  LCD_PORT(var);	\
  _delay_us(50);	\
  LCD_rs_low;		\
  _delay_us(50);	\
  LCD_en_high;		\
  _delay_us(50);	\
  LCD_en_low;		\
  LCD_rw_high;		\
  _delay_us(50)

# define LCD_idle_drive \
  LCD_PORT(0);		\
  LCD_rs_low;		\
  LCD_en_low;		\
  LCD_rw_high;		\
  _delay_us(100)

#endif // #if LCD_DPORT_SIZE

#define LCD_refresh()        // Nothing

#endif // #if UNIT_TEST

// Not working
//#define LCD_CLRSCR  LCD_cmd(LCD_CMD_CLRSCR)
#if LCD2004
#define LCD_CLRSCR				\
  LCD_CLRLINE(3); LCD_CLRLINE(2); LCD_CLRLINE(1); LCD_CLRLINE(0)
#else
#define LCD_CLRSCR				\
  LCD_CLRLINE(1); LCD_CLRLINE(0)
#endif

void LCD_CLRLINE(uint8_t line);
void LCD_WR(char *str);
void LCD_WR_N(uint8_t *str, uint8_t len);
void LCD_WR_P(const char *str);
void LCD_WR_NP(const char *str, uint8_t len);
void LCD_BUSY(void);

void LCD_PUT_UINT8X(uint8_t ch);
void LCD_PUT_UINT16X(uint16_t ch);
void LCD_PUT_UINT(uint32_t val);
void LCD_PUT_FLOAT(uint32_t val);
void LCD_PUTCH(uint8_t var);

#define LCD_ALERT(S)      do {			\
  lcd_alert(S);					\
  } while (0)
#define LCD_ALERT_N(S, N)  do {			\
  lcd_alert_n(S, N);				\
  KBD_GETCH_ALERT_N;				\
  } while (0)

void lcd_alert(const char *str);
void lcd_alert_n(const char *str, uint32_t n);

void LCD_init(void);
void lcdD(uint32_t var);
void lcdFd(uint32_t var);

#endif
