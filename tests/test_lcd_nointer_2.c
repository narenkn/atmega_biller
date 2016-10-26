#include <avr/io.h>
#include <util/delay.h>

# define LCD_CMD_DISON_CURON_BLINKON 0x0F /* LCD ON, Cursor ON, Cursor blinking ON */
# define LCD_CMD_CLRSCR        0x01   /* Clear screen */
# define LCD_CMD_HOME          0x02   /* Return home */
# define LCD_CMD_DEC_CUR       0x04   /* Decrement cursor */
# define LCD_CMD_INC_CUR       0x06   /* Increment cursor */
# define LCD_CMD_DISON_CURON   0x0E   /* Display ON ,Cursor ON */
# define LCD_CMD_CUR_10        0x80   /* Force cursor to the beginning of 1st line */
# define LCD_CMD_CUR_20        0xC0   /* Force cursor to the beginning of 2nd line */
# define LCD_ACT_LINE2         0x3C   /* Activate second line */
# define LCD_CMD_2LINE_5x7     0x28   /* Use 2 lines and 5×7 matrix */

#define LCD_en_high  PORTB |= _BV(7)
#define LCD_en_low   PORTB &= ~_BV(7)
#define LCD_rs_high  PORTB |= _BV(5)
#define LCD_rs_low   PORTB &= ~_BV(5)
#define LCD_rw_high  PORTB |= _BV(6)
#define LCD_rw_low   PORTB &= ~_BV(6)
#define LCD_bl_on    PORTG |= _BV(2)
#define LCD_bl_off   PORTG &= ~_BV(2)
#define LCD_WAS_ON  (PORTD&0x10)

# define LCD_PORT(val)  PORTD &= ~0xF0; PORTD |= val & 0xF0; PORTA &= 0xF; PORTA |= val

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

int
main(void)
{
  DDRD = 0x10;
  PORTD = 0x10;

  DDRA = 0x0F;
  DDRC = 0xC0;

#if 4 == LCD_DPORT_SIZE
  _delay_ms(100);
  LCD_wrnib(3);
  _delay_ms(100);
  LCD_wrnib(3);
  _delay_ms(10);
  LCD_wrnib(3);
  _delay_ms(10);
  LCD_wrnib(2);
  _delay_ms(10);
#endif

  /*  Function set: 2 Line, 8-bit, 5x7 dots */
  LCD_cmd(LCD_CMD_2LINE_5x7);
  LCD_cmd(LCD_CMD_2LINE_5x7);
  LCD_cmd(LCD_CMD_2LINE_5x7);
  LCD_cmd(LCD_CMD_2LINE_5x7);
  LCD_cmd(LCD_CMD_2LINE_5x7);
  _delay_ms(100);

  /* Display on, Curson blinking command */
  LCD_cmd(LCD_CMD_DISON_CURON_BLINKON);
  _delay_ms(100);

  /* Clear LCD */
  LCD_cmd(LCD_CMD_CLRSCR);
  LCD_cmd(LCD_CMD_CLRSCR);
  LCD_cmd(LCD_CMD_CLRSCR);
  _delay_ms(100);

  /* Entry mode, auto increment with no shift */
  LCD_cmd(LCD_CMD_INC_CUR);
  LCD_cmd(LCD_CMD_INC_CUR);
  _delay_ms(100);

  while (1) {
    PORTD |= 0x10;
    LCD_wrchar('H');
    LCD_wrchar('e');
    LCD_wrchar('l');
    LCD_wrchar('l');
    LCD_wrchar('o');
    _delay_ms(1000);
    PORTD &= ~0x10;
    _delay_ms(1000);
  }
  return 0;
}
