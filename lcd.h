#ifndef LCD__H
#define LCD__H

#define LCD_MAX_ROW            2
#define LCD_MAX_COL           16

#if 8 == LCD_DPORT_SIZE
# define LCD_PORT(val)  P1 = val
#elif 4 == LCD_DPORT_SIZE
# define LCD_PORT(val)				\
  PORTA = (PORTA & ~0xF) | ((val) & 0xF)
#endif
#define LCD_en_high  PORTC |= 0x80
#define LCD_en_low   PORTC &= ~0x80
#define LCD_rs_high  PORTC |= 0x40
#define LCD_rs_low   PORTC &= ~0x40
#define LCD_bl_on    PORTD |= 0x10
#define LCD_bl_off   PORTD &= ~0x10
#define LCD_WAS_ON  (PORTD&0x10)

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
# define LCD_CMD_CUR_20        0xC0   /* Force cursor to the beginning of 2nd line */
# define LCD_ACT_LINE2         0x3C   /* Activate second line */

#if 8 == LCD_DPORT_SIZE

# define LCD_CMD_2LINE_5x7     0x38   /* Use 2 lines and 5×7 matrix */

# define LCD_cmd(var)   \
  LCD_PORT(var);	\
  LCD_rs = 0;		\
  LCD_en = 1;		\
  LCD_en = 0;		\
  _delay_ms(2)

# define LCD_idle_drive \
  LCD_PORT(0);		\
  LCD_rs = 0;		\
  LCD_en = 0

# define LCD_wrchar(var)\
  LCD_PORT(var);	\
  LCD_rs = 1;		\
  LCD_en = 1;		\
  LCD_en = 0;		\
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
//#define LCD_CLRSCR				\
//  LCD_cmd(LCD_CMD_CLRSCR)
#define LCD_CLRSCR				\
  LCD_CLRLINE(1); LCD_CLRLINE(0)

# define LCD_wrnib(var)				\
  LCD_PORT(var);				\
  _delay_us(50);				\
  LCD_rs_high;					\
  _delay_us(50);				\
  LCD_en_high;					\
  _delay_us(50);				\
  LCD_en_low

void LCD_CLRLINE(uint8_t line);
void LCD_WR(char *str);
void LCD_WR_N(char *str, uint8_t len);
void LCD_WR_P(const uint8_t *str);
void LCD_WR_NP(const uint8_t *str, uint8_t len);

void LCD_PUT_UINT8X(uint8_t ch);
void LCD_PUT_UINT16X(uint16_t ch);
void LCD_PUT_UINT(uint32_t val);

#define LCD_PUTCH   LCD_wrchar

#define LCD_ALERT(str)				\
  LCD_CLRLINE(0);				\
  LCD_WR_P((const uint8_t *)str);		\
  LCD_refresh();				\
  KBD_GETCH

#define LCD_ALERT_16N(str, n)			\
  LCD_CLRLINE(0);				\
  LCD_WR_P((const uint8_t *)str);		\
  LCD_PUT_UINT16X(n);				\
  LCD_refresh();				\
  KBD_GETCH

extern void LCD_init(void);

#endif
