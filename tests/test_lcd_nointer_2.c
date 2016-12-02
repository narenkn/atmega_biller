#include "lcd.c"
#include "i2c.c"

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
