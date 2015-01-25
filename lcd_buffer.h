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

#define LCD_PROP_NOECHO_L2  (1<<0)
#define LCD_PROP_DIRTY      (1<<1)

#ifdef  UNIT_TEST

# define LCD_CMD_DISON_CURON_BLINKON assert(0)
# define LCD_CMD_CLRSCR        LCD_CLRSCR
# define LCD_CMD_HOME          lcd_buf_p = (uint8_t *)lcd_buf
# define LCD_CMD_DEC_CUR       assert(0)
# define LCD_CMD_INC_CUR       assert(0)
# define LCD_CMD_DISON_CURON   assert(0)
# define LCD_CMD_CUR_10        lcd_buf_p = (uint8_t *)lcd_buf
# define LCD_CMD_CUR_20        lcd_buf_p = &(lcd_buf[1][0])
# define LCD_CMD_2LINE_5x7     assert(0)
# define LCD_ACT_LINE2         assert(0)

# define LCD_cmd(CMD)          CMD

# define LCD_idle_drive

# define LCD_wrchar            LCD_PUTCH

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

# define LCD_CMD_2LINE_5x7     0x38   /* Use 2 lines and 5�7 matrix */

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

# define LCD_CMD_2LINE_5x7     0x28   /* Use 2 lines and 5�7 matrix */

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

#endif // #ifdef UNIT_TEST

#if  !LCD_USE_FUNCTIONS

#define LCD_CLRSCR {				\
  uint16_t ui1_t;				\
  lcd_buf_p = (uint8_t *)lcd_buf;		\
  for (ui1_t=0; ui1_t<(LCD_MAX_COL*LCD_MAX_ROW); ui1_t++) {	\
    lcd_buf_p[0] = ' ';				\
    lcd_buf_p++;				\
  }						\
  lcd_buf_p = (uint8_t *)lcd_buf;		\
  lcd_buf_prop |= LCD_PROP_DIRTY;		\
}

#define LCD_CUR_POS_P lcd_buf_p
#define LCD_CUR_POS   (&(lcd_buf[0][0])-lcd_buf_p)

#define LCD_WR_LINE(x, y, str)  {				\
    uint8_t ui1_t, ui2_t;					\
    lcd_buf_p = lcd_buf + (x % LCD_MAX_ROW);			\
    lcd_buf_p += (y % LCD_MAX_COL);				\
    for (ui1_t=0, ui2_t=0; (ui1_t<(LCD_MAX_COL-y)); ui1_t++) {	\
      if (0 == ((char *)str)[ui2_t]) {				\
	lcd_buf_p[0] = ' ';					\
      } else {							\
	lcd_buf_p[0] = ((char *)str)[ui2_t];			\
	ui2_t++;						\
      }								\
      lcd_buf_p++;						\
    }								\
    lcd_buf_prop |= LCD_PROP_DIRTY;				\
}

#define LCD_WR_LINE_P(x, y, str)  {				\
    uint8_t ui1_t, ui2_t;					\
    lcd_buf_p = lcd_buf + (x % LCD_MAX_ROW);			\
    lcd_buf_p += (y % LCD_MAX_COL);				\
    for (ui1_t=0, ui2_t=0; (ui1_t<(LCD_MAX_COL-y)); ui1_t++) {	\
      if (0 == pgm_read_byte(str+ui2_t)) {			\
	lcd_buf_p[0] = ' ';					\
      } else {							\
	lcd_buf_p[0] = pgm_read_byte(str+ui2_t);		\
	ui2_t++;						\
      }								\
      lcd_buf_p++;						\
    }								\
    lcd_buf_prop |= LCD_PROP_DIRTY;				\
  }

#define LCD_WR_LINE_N(x, y, str, len)  {				\
    uint8_t ui1_t, len_s;						\
    lcd_buf_p = lcd_buf + (x % LCD_MAX_ROW);				\
    lcd_buf_p += (y % LCD_MAX_COL);					\
    len_s = (len) % (LCD_MAX_COL+1-y);					\
    for (ui1_t=0; (0 != (str+ui1_t)[0]) && (ui1_t<len_s); ui1_t++) {	\
      lcd_buf_p[0] = (str+ui1_t)[0];					\
      lcd_buf_p++;							\
    }									\
    for (; 0 != (ui1_t<(LCD_MAX_COL-y)); ui1_t++) {			\
      lcd_buf_p[0] = ' ';						\
      lcd_buf_p++;							\
    }									\
    lcd_buf_prop |= LCD_PROP_DIRTY;					\
  }

#define LCD_WR_LINE_NP(x, y, str, len)  {	\
    uint8_t ui1_t, len_s;			\
    lcd_buf_p = lcd_buf + (x % LCD_MAX_ROW);	\
    lcd_buf_p += (y % LCD_MAX_COL);		\
    len_s = (len) % (LCD_MAX_COL+1-y);		\
    for (ui1_t=0; (ui1_t<len_s); ui1_t++) {	\
      lcd_buf_p[0] = pgm_read_byte(str+ui1_t);	\
      if (0 == lcd_buf_p[0]) break;		\
      lcd_buf_p++;				\
    }						\
    for (; ui1_t < (LCD_MAX_COL-y); ui1_t++) {	\
      lcd_buf_p[0] = ' ';			\
      lcd_buf_p++;				\
    }						\
    lcd_buf_prop |= LCD_PROP_DIRTY;		\
  }

#define LCD_WR_LINE_N_EE24XX(x, y, str, len)  {			\
    uint8_t ui1_t, len_s;					\
    lcd_buf_p = lcd_buf + (x % LCD_MAX_ROW);			\
    lcd_buf_p += (y % LCD_MAX_COL);				\
    len_s = (len) % (LCD_MAX_COL-y);				\
    ee24xx_read_bytes((uint16_t)(str), lcd_buf_p, len_s+1);	\
    lcd_buf_prop |= LCD_PROP_DIRTY;				\
  }

#define LCD_POS(x, y)				\
  lcd_buf_p = lcd_buf + (x % LCD_MAX_ROW);	\
  lcd_buf_p += (y % LCD_MAX_COL)

#define LCD_WR_N(str, len) {					\
    uint8_t ui1_t, len_s;					\
    len_s = (len) % (LCD_MAX_COL+1);				\
    for (ui1_t=0; (0 != str[ui1_t]) && (ui1_t<len_s); ui1_t++) {	\
      lcd_buf_p[0] = str[ui1_t];				\
      if ((lcd_buf_p-lcd_buf[0]) >= (LCD_MAX_COL*LCD_MAX_ROW))	\
	break;							\
      lcd_buf_p++;						\
    }								\
    for (; (ui1_t<len_s); ui1_t++) {				\
      lcd_buf_p[0] = ' ';					\
      if ((lcd_buf_p-lcd_buf[0]) >= (LCD_MAX_COL*LCD_MAX_ROW))	\
	break;							\
      lcd_buf_p++;						\
    }								\
    lcd_buf_prop |= LCD_PROP_DIRTY;				\
  }

#define LCD_WR_P(str) {						\
    uint8_t ui1_t, ui2_t;					\
    for (ui1_t=0; (ui1_t < LCD_MAX_COL); ui1_t++) {		\
      if ((lcd_buf_p-lcd_buf[0]) >= (LCD_MAX_COL*LCD_MAX_ROW))	\
	break;							\
      ui2_t = pgm_read_byte(str+ui1_t);				\
      if (0 == ui2_t)						\
	break;							\
      lcd_buf_p[0] = ui2_t;					\
      lcd_buf_p++;						\
    }								\
    lcd_buf_prop |= LCD_PROP_DIRTY;				\
  }

#define LCD_PUT_UINT8X(ch) do {					\
    uint8_t ui2_a = (ch>>4) & 0xF;				\
    if ((lcd_buf_p-lcd_buf[0]) >= (LCD_MAX_COL*LCD_MAX_ROW))	\
      break;							\
    lcd_buf_p[0] = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;		\
    lcd_buf_p++;						\
    if ((lcd_buf_p-lcd_buf[0]) >= (LCD_MAX_COL*LCD_MAX_ROW))	\
      break;							\
    ui2_a = ch & 0xF;						\
    lcd_buf_p[0] = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;		\
    lcd_buf_prop |= LCD_PROP_DIRTY;				\
    lcd_buf_p++;						\
  } while (0)

#define LCD_PUT_UINT16X(ch) do {				\
    uint8_t ui2_a = (ch>>12) & 0xF;				\
    if ((lcd_buf_p-lcd_buf[0]) >= (LCD_MAX_COL*LCD_MAX_ROW))	\
      break;							\
    lcd_buf_p[0] = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;		\
    lcd_buf_p++;						\
    if ((lcd_buf_p-lcd_buf[0]) >= (LCD_MAX_COL*LCD_MAX_ROW))	\
      break;							\
    ui2_a = (ch>>8) & 0xF;					\
    lcd_buf_p[0] = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;		\
    lcd_buf_p++;						\
    if ((lcd_buf_p-lcd_buf[0]) >= (LCD_MAX_COL*LCD_MAX_ROW))	\
      break;							\
    ui2_a = (ch>>4) & 0xF;					\
    lcd_buf_p[0] = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;		\
    lcd_buf_p++;						\
    if ((lcd_buf_p-lcd_buf[0]) >= (LCD_MAX_COL*LCD_MAX_ROW))	\
      break;							\
    ui2_a = ch & 0xF;						\
    lcd_buf_p[0] = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;		\
    lcd_buf_prop |= LCD_PROP_DIRTY;				\
    lcd_buf_p++;						\
  } while (0)

#define LCD_PUTCH(ch) do {					\
    if ((lcd_buf_p-lcd_buf[0]) >= (LCD_MAX_COL*LCD_MAX_ROW))	\
      break;							\
    lcd_buf_p[0] = ch;						\
    lcd_buf_p++;						\
    lcd_buf_prop |= LCD_PROP_DIRTY;				\
  } while (0)

#define LCD_WR_SPRINTF(x, y, BUF, FMT, N) do {				\
    uint8_t ui8_1;							\
    sprintf(BUF, FMT, N);						\
    lcd_buf_p = lcd_buf + (x % LCD_MAX_ROW);				\
    lcd_buf_p += (y % LCD_MAX_COL);					\
    for (ui8_1=0; (0!=BUF[ui8_1]) && (ui8_1 < (LCD_MAX_COL-y)); ui8_1++)  { \
      lcd_buf_p[0] = BUF[ui8_1];					\
      lcd_buf_p++;							\
    }									\
    lcd_buf_prop |= LCD_PROP_DIRTY;					\
  } while (0)

#define LCD_ALERT(str)				\
  LCD_WR_LINE_P(0, 0, str);			\
  LCD_refresh();				\
  KBD_GETCH

#define LCD_ALERT_16N(str, n)			\
  LCD_WR_LINE_P(0, 0, str);			\
  LCD_POS(0, 12);				\
  LCD_PUT_UINT16X(n);				\
  LCD_refresh();				\
  KBD_GETCH

#else /* #if LCD_USE_FUNCTIONS */

# define LCD_wrnib(var)				\
  LCD_PORT(var);				\
  _delay_us(50);				\
  LCD_rs_high;					\
  _delay_us(50);				\
  LCD_en_high;					\
  _delay_us(50);				\
  LCD_en_low

void lcd_clrscr();
#define LCD_CLRSCR lcd_clrscr()

#define LCD_CUR_POS_P lcd_buf_p
#define LCD_CUR_POS   (lcd_buf[0]-lcd_buf_p)

#define LCD_POS(x, y)				\
  lcd_buf_p = lcd_buf + (x % LCD_MAX_ROW);	\
  lcd_buf_p += (y % LCD_MAX_COL);

#ifndef UNIT_TEST

void LCD_WR_LINE(uint8_t x, uint8_t y, uint16_t str);

void LCD_WR_LINE_P(uint8_t x, uint8_t y, uint16_t str);

void LCD_WR_LINE_N(uint8_t x, uint8_t y, uint8_t *str, uint8_t len);

void LCD_WR_LINE_NP(uint8_t x, uint8_t y, uint8_t *str, uint8_t len);

void LCD_WR_LINE_N_EE24XX(uint8_t x, uint8_t y, uint16_t str, uint8_t len);

void LCD_WR_N(uint8_t *str, uint8_t len);

void LCD_WR_P(uint16_t str);

void LCD_PUT_UINT8X(uint8_t ch);
void LCD_PUT_UINT16X(uint16_t ch);
void LCD_PUTCH(uint8_t ch);
void LCD_WR_SPRINTF(uint8_t x, uint8_t y, uint8_t *BUF, uint8_t *FMT, uint8_t N);

#endif

#define LCD_ALERT(str)				\
  LCD_WR_LINE_P(0, 0, str);			\
  LCD_refresh();				\
  KBD_GETCH

#define LCD_ALERT_16N(str, n)			\
  LCD_WR_LINE_P(0, 0, str);			\
  LCD_POS(0, 12);				\
  LCD_PUT_UINT16X(n);				\
  LCD_refresh();				\
  KBD_GETCH

#endif

extern uint8_t lcd_buf_prop;
extern uint8_t *lcd_buf_p;
extern uint8_t lcd_buf[LCD_MAX_ROW][LCD_MAX_COL];
extern void LCD_init(void);
extern void LCD_refresh(void);

#endif
