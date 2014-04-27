
#define assert(x)

#include "lcd.c"
#include <avr/interrupt.h>

volatile uint8_t count;

ISR(TIMER0_OVF_vect)
{
  uint8_t ui2, *ui3_p;

  /* Increment our variable */
  count++;
  if (count < 61) {
    return;
  }

  /**************************************************/
  /*****************   LCD   ************************/
  /**************************************************/
  /* Display on LCD */
  LCD_cmd(LCD_CMD_CLRSCR);
  LCD_cmd(LCD_CMD_CUR_10);
  ui3_p = (uint8_t*) lcd_buf;
  for (ui2=0; ui2<LCD_MAX_COL; ui2++) {
    LCD_wrchar(ui3_p[0]);
    ui3_p++;
  }
  LCD_cmd(LCD_CMD_CUR_20);
  for (ui2=0; ui2<LCD_MAX_COL; ui2++) {
    if ((lcd_buf_prop & LCD_PROP_NOECHO_L2) && (' ' != ui3_p[0])) {
      LCD_wrchar('*');
    } else {
      LCD_wrchar(ui3_p[0]);
    }
    ui3_p++;
  }
}

void
main()
{
  uint16_t ui1;

  // Prescaler = FCPU/1024
  TCCR0|=(1<<CS02)|(1<<CS00);

  //Enable Overflow Interrupt Enable
  TIMSK|=(1<<TOIE0);

  //Initialize Counter
  TCNT0=0;

  //Initialize our varriable
  count=0;

  //Port C[3,2,1,0] as out put
  DDRC|=0x0F;

  //Enable Global Interrupts
  sei();

  LCD_init();

  DDRD |= 0x10;
  PORTD |= 0x10;

  _delay_ms(2);
  LCD_CLRSCR;

  LCD_WR_LINE(0, 0, "Hello World 7");
  for (ui1=0; ui1<0x4FF; ui1++)
    _delay_ms(2000);
  LCD_WR_LINE(1, 0, "Hello World 8");
  for (ui1=0; ui1<0x4FF; ui1++)
    _delay_ms(2000);

  //Infinite loop
  while(1);
}
