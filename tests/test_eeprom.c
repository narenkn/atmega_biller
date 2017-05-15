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
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("EEPROM Testing:"));
  _delay_ms(1000);

  uint8_t data[8] = {0x1, 0x3, 0x5, 0x8, 0x13, 0x21, 0x34, 0x55};
  uint8_t dread[8];
  uint8_t ui1, errors=0;

  /* Test 1 */
  ee24xx_write_bytes(0, data, 4);
  ee24xx_read_bytes(0, dread, 4);
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("24c512 LowAddrT"));
  for (ui1=0; ui1<4; ui1++) {
    LCD_CLRLINE(1);
    LCD_PUT_UINT8X(ui1);
    LCD_PUTCH(':');
    LCD_PUT_UINT8X(data[ui1]);
    LCD_WR_P(PSTR(" Vs "));
    LCD_PUT_UINT8X(dread[ui1]);
    _delay_ms(1000);
    if (data[ui1] != dread[ui1]) errors++;
  }

  /* Test 2 */
  ee24xx_write_bytes(EEPROM_MAX_ADDRESS-1, data+4, 4);
  ee24xx_read_bytes(EEPROM_MAX_ADDRESS-1, dread+4, 4);
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("24c512 MaxAddrT"));
  for (ui1=4; ui1<8; ui1++) {
    LCD_CLRLINE(1);
    LCD_PUT_UINT8X(ui1);
    LCD_PUTCH(':');
    LCD_PUT_UINT8X(data[ui1]);
    LCD_WR_P(PSTR(" Vs "));
    LCD_PUT_UINT8X(dread[ui1]);
    _delay_ms(1000);
    if (data[ui1] != dread[ui1]) errors++;
  }

  LCD_CLRLINE(1);
  LCD_WR_P(PSTR("Errors: "));
  LCD_PUT_UINT(errors);

  //Infinite loop
  while(1);

  return 0;
}
