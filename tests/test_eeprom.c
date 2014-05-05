
#define assert(x)

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "lcd.c"
#include "i2c.c"

int
main()
{
  _delay_ms(1000);
  LCD_init();
  i2c_init();

  LCD_bl_on;
  LCD_WR_LINE(0, 0, "EEPROM Testing:");
  LCD_refresh();
  _delay_ms(1000);

  uint8_t data[8] = {0x1, 0x3, 0x5, 0x8, 0x13, 0x21, 0x34, 0x55};
  uint8_t dread[8];
  uint8_t ui1;
  ee24xx_write_bytes(0, data, 8);
  ee24xx_read_bytes(0, dread, 8);
  for (ui1=0; ui1<8; ui1++) {
    LCD_POS(1, 0);
    LCD_PUT_UINT8X(ui1);
    LCD_POS(1, 4);
    LCD_PUT_UINT8X(dread[ui1]);
    _delay_ms(1000);
    LCD_refresh();
  }

  //Infinite loop
  while(1);

  return 0;
}
