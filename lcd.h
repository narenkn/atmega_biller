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

#endif // #ifdef UNIT_TEST

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

#define LCD_WR_LINE(x, y, str)  {	\
  uint8_t ui1_t, ui2_t;				\
  lcd_buf_p = &(lcd_buf[x][y]);			\
  for (ui1_t=0, ui2_t=0; (ui1_t<LCD_MAX_COL); ui1_t++) {	\
    if (0 == ((char *)str)[ui2_t]) {			\
      lcd_buf_p[0] = ' ';		\
    } else {				\
      lcd_buf_p[0] = ((char *)str)[ui2_t];	\
      ui2_t++;					\
    }					\
    lcd_buf_p++;						\
    if ((ui1_t+1)<LCD_MAX_COL)					\
      assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));	\
  }					\
  lcd_buf_prop |= LCD_PROP_DIRTY;		\
}

#define LCD_WR_LINE_N(x, y, str, len)  {	\
  uint8_t ui1_t;			        \
  lcd_buf_p = &(lcd_buf[x][y]);			\
  for (ui1_t=0; (0 != (str+ui1_t)[0]) && (ui1_t<len) && (ui1_t < LCD_MAX_COL); ui1_t++) { \
      lcd_buf_p[0] = (str+ui1_t)[0];		\
      lcd_buf_p++;				\
      if ((ui1_t+1)<len)			\
	assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));	\
  }						\
  for (; 0 != (ui1_t<LCD_MAX_COL); ui1_t++) {	\
    lcd_buf_p[0] = ' ';				\
    lcd_buf_p++;				\
  }						\
  lcd_buf_prop |= LCD_PROP_DIRTY;		\
}

#define LCD_WR_LINE_NP(x, y, str, len)  {	\
  uint8_t ui1_t;			        \
  lcd_buf_p = &(lcd_buf[x][y]);			\
  for (ui1_t=0; (ui1_t<len); ui1_t++) {		\
    lcd_buf_p[0] = pgm_read_byte(((char *)str)+ui1_t);	\
    if (0 == lcd_buf_p[0]) break;		\
    lcd_buf_p++;				\
    if ((ui1_t+1)<len)				\
      assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));	\
  }						\
  for (; ui1_t < LCD_MAX_COL; ui1_t++) {	\
    lcd_buf_p[0] = ' ';				\
    lcd_buf_p++;				\
  }						\
  lcd_buf_prop |= LCD_PROP_DIRTY;		\
}

#define LCD_WR_LINE_N_EE24XX(x, y, str, len)  {	\
  uint8_t ui1_t;			        \
  lcd_buf_p = &(lcd_buf[x][y]);			\
  ee24xx_read_bytes((uint16_t)(str), lcd_buf_p, len);			\
  for (ui1_t = len, lcd_buf_p+=len; ui1_t < LCD_MAX_COL; ui1_t++) {	\
    lcd_buf_p[0] = ' ';				\
    lcd_buf_p++;				\
  }						\
  lcd_buf_prop |= LCD_PROP_DIRTY;		\
}

#define LCD_POS(x, y)				\
  lcd_buf_p = &(lcd_buf[x][y])

#define LCD_WR_N(str, len) {					\
  uint8_t ui1_t;						\
  for (ui1_t=0; (0 != str[ui1_t]) && (ui1_t<len); ui1_t++) {	\
    lcd_buf_p[0] = str[ui1_t];					\
    lcd_buf_p++;						\
    if ((ui1_t+1)<len)						\
      assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));	\
  }								\
  for (; (ui1_t<len); ui1_t++) {				\
    lcd_buf_p[0] = ' ';						\
    lcd_buf_p++;						\
    if ((ui1_t+1)<len)						\
      assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));	\
  }								\
  lcd_buf_prop |= LCD_PROP_DIRTY;				\
}

#define LCD_WR(str) {	 \
  uint8_t ui1_t;			 \
  for (ui1_t=0; 0 != str[ui1_t]; ui1_t++) {	 \
    lcd_buf_p[0] = str[ui1_t];	 \
    lcd_buf_p++;		 \
    assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));	\
  }				 \
  lcd_buf_prop |= LCD_PROP_DIRTY;		\
}

#define LCD_PUT_UINT8X(ch) {			\
  uint8_t ui2_a = (ch>>4) & 0xF;			\
  lcd_buf_p[0] = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;	\
  lcd_buf_p++;					\
  assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL)); \
  ui2_a = ch & 0xF;				\
  lcd_buf_p[0] = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;	\
  lcd_buf_p++;					\
  lcd_buf_prop |= LCD_PROP_DIRTY;		\
  assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL)); \
}

#define LCD_PUT_UINT16X(ch) {			\
  uint8_t ui2_a = (ch>>12) & 0xF;			\
  lcd_buf_p[0] = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;	\
  lcd_buf_p++;					\
  assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL)); \
  ui2_a = (ch>>8) & 0xF;				\
  lcd_buf_p[0] = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;	\
  lcd_buf_p++;					\
  assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL)); \
  ui2_a = (ch>>4) & 0xF;				\
  lcd_buf_p[0] = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;	\
  lcd_buf_p++;					\
  assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL)); \
  ui2_a = ch & 0xF;				\
  lcd_buf_p[0] = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;	\
  lcd_buf_p++;					\
  lcd_buf_prop |= LCD_PROP_DIRTY;		\
  assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL)); \
}

#define LCD_PUTCH(ch) {				\
  lcd_buf_p[0] = ch;				\
  lcd_buf_p++;					\
  lcd_buf_prop |= LCD_PROP_DIRTY;		\
  assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL)); \
}

#define LCD_WR_SPRINTF(x, y, BUF, FMT, N) {	\
  uint8_t ui8_1;				\
  sprintf(BUF, FMT, N);				\
  lcd_buf_p = &(lcd_buf[x][y]);			\
  for (ui8_1=0; 0!=BUF[ui8_1]; ui8_1++)  {	\
    lcd_buf_p[0] = BUF[ui8_1];			\
    lcd_buf_p++;				\
  }						\
}

#define LCD_ALERT(str)	  \
  LCD_WR_LINE(0, 0, str)

#define LCD_ALERT_16N(str, n) \
  LCD_WR_LINE(0, 0, str);     \
  LCD_PUT_UINT16X(n)

extern uint8_t lcd_buf_prop;
extern uint8_t *lcd_buf_p;
extern uint8_t lcd_buf[LCD_MAX_ROW][LCD_MAX_COL];
extern void LCD_init(void);
extern void LCD_refresh(void);

#endif
