#ifndef LCD__H
#define LCD__H

#define LCD_MAX_ROW            2
#define LCD_MAX_COL           16

#if 8 == LCD_DPORT_SIZE
# define LCD_PORT(val)  PORTD &= ~0xF0; PORTD |= val & 0xF0; PORTA &= 0xF; PORTA |= val
#elif 4 == LCD_DPORT_SIZE
# define LCD_PORT(val)				\
  PORTA = (PORTA & ~0xF) | ((val) & 0xF)
#endif
#define LCD_en_high  PORTB |= _BV(7)
#define LCD_en_low   PORTB &= ~_BV(7)
#define LCD_rs_high  PORTB |= _BV(5)
#define LCD_rs_low   PORTB &= ~_BV(5)
#define LCD_rw_high  PORTB |= _BV(6)
#define LCD_rw_low   PORTB &= ~_BV(6)
#define LCD_bl_on    PORTG |= _BV(2)
#define LCD_bl_off   PORTG &= ~_BV(2)
#define LCD_WAS_ON  (PORTG & _BV(2))

#ifdef  UNIT_TEST

# define LCD_CMD_DISON_CURON_BLINKON assert(0)
# define LCD_CMD_CLRSCR        lcd_clrscr()
# define LCD_CMD_HOME          lcd_buf_p = (uint8_t *)lcd_buf
# define LCD_CMD_DEC_CUR
# define LCD_CMD_INC_CUR       assert(0)
# define LCD_CMD_DISON_CURON   assert(0)
# define LCD_CMD_CUR_10        lcd_buf_p = (uint8_t *)lcd_buf
# define LCD_CMD_CUR_20        lcd_buf_p = &(lcd_buf[1][0])
# define LCD_CMD_2LINE_5x7     assert(0)
# define LCD_ACT_LINE2         assert(0)
# define LCD_cmd(CMD)          CMD
# define LCD_idle_drive

#define LCD_refresh()          // FIXME: put debug info

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
# define LCD_ACT_LINE2         0x3C   /* Activate second line */

#if 8 == LCD_DPORT_SIZE

# define LCD_CMD_2LINE_5x7     0x38   /* Use 2 lines and 5×7 matrix */

# define LCD_cmd(var)   \
  LCD_PORT(var);	\
  LCD_rs_low;		\
  LCD_en_high;		\
  LCD_en_low;		\
  _delay_ms(2)

# define LCD_idle_drive \
  LCD_PORT(0);		\
  LCD_rs_low;		\
  LCD_en_low

# define LCD_wrchar(var)\
  LCD_PORT(var);	\
  LCD_rs_high;		\
  LCD_en_high;		\
  LCD_en_low;		\
  _delay_ms(2)

#elif 4 == LCD_DPORT_SIZE

# define LCD_CMD_2LINE_5x7     0x28   /* Use 2 lines and 5×7 matrix */

# define LCD_wrnib(var)	\
  LCD_PORT(var);	\
  _delay_us(50);	\
  LCD_rs_high;		\
  _delay_us(50);	\
  LCD_en_high;		\
  _delay_us(50);	\
  LCD_en_low

# define LCD_wrchar(var)\
  LCD_wrnib(var>>4);	\
  _delay_us(50);	\
  LCD_wrnib(var);	\
  _delay_us(50)

# define LCD_cmd(var)   \
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
  _delay_us(50)

# define LCD_idle_drive \
  LCD_PORT(0);		\
  LCD_rs_low;		\
  LCD_en_low;		\
  _delay_us(100)

#endif // #if LCD_DPORT_SIZE

#define LCD_refresh()        // Nothing

#endif // #ifdef UNIT_TEST

// Not working
//#define LCD_CLRSCR  LCD_cmd(LCD_CMD_CLRSCR)
#define LCD_CLRSCR				\
  LCD_CLRLINE(1); LCD_CLRLINE(0)

void LCD_CLRLINE(uint8_t line);
void LCD_WR(char *str);
void LCD_WR_N(uint8_t *str, uint8_t len);
void LCD_WR_P(const char *str);
void LCD_WR_NP(const char *str, uint8_t len);

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
  KBD_GETCH;					\
  } while (0)

void lcd_alert(const char *str);
void lcd_alert_n(const char *str, uint32_t n);

void LCD_init(void);
void lcdD(uint32_t var);
void lcdFd(uint32_t var);

#endif
