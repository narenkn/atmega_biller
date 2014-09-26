#include "test_common.c"

/* test setting set routines
 */

/* place to store items */
struct item all_items[ITEM_MAX];

/* Convert a int to string */
void
int2str(char *str, uint32_t ui, uint32_t *idx)
{
  if (ui > 9) {
    int2str(str, ui/10, idx);
  }
  str[*idx] = '0' + (ui%10);
  (*idx)++;
  str[*idx] = 0;
}

int
main()
{
  uint32_t errors;
  uint8_t  ui8_1, ui8_2;
  uint32_t loop, ui32_1, ui32_2;
  uint16_t ui16_1, ui16_2, ui16_3;
  uint8_t quest[LCD_MAX_COL];

  for (loop=0; loop<1; loop++) {
    /* setup the question */
    ui8_2 = rand() % (LCD_MAX_COL-1);
    ui8_2++;
    for (ui8_1=0; ui8_1<LCD_MAX_COL; ui8_1++)
      quest[ui8_1] = (ui8_1 < ui8_2) ? 'a' + (rand()%26) : 0;
    /* size of string to get */
    ui16_2 = rand() % 500;
    ui16_2++;
    /* address to store */
    ui16_1 = rand() % AVR_EEPROM_SIZE;
    if ((ui16_1+ui16_2) >= AVR_EEPROM_SIZE)
      ui16_1 -= ui16_2;
    /* setup the string */
    for (ui16_3=0; ui16_3<ui16_2; ui16_3++) {
      inp[ui16_3] = 'a' + rand() % 26;
    }
    INIT_TEST_KEYS(inp);
    menuSettingString(ui16_1, quest, ui8_1);
    /* check question */
    assert(0 == strncmp(lcd_buf[0], quest, MENU_PROMPT_LEN));
    /* check stored value */
    assert(0 == strncmp(inp, _avr_eeprom[ui16_1], ui16_2));
  }

  for (loop=0; loop<0; loop++) {
    /* setup the question */
    ui8_2 = rand() % (LCD_MAX_COL-1);
    ui8_2++;
    for (ui8_1=0; ui8_1<LCD_MAX_COL; ui8_1++)
      quest[ui8_1] = (ui8_1 < ui8_2) ? 'a' + (rand()%26) : 0;
    /* address to store */
    ui16_1 = rand() % AVR_EEPROM_SIZE;
    /* rand value to get */
    ui8_1 = rand();
    ui32_1 = 0;
    int2str(inp, ui8_1, ui32_1)
    /* */
    INIT_TEST_KEYS(inp);
    menuSettingUint8(ui16_1, quest);
    /* check */
    assert(ui8_1 == _avr_eeprom[ui16_1]);
  }

  for (loop=0; loop<0; loop++) {
    /* setup the question */
    ui8_2 = rand() % (LCD_MAX_COL-1);
    ui8_2++;
    for (ui8_1=0; ui8_1<LCD_MAX_COL; ui8_1++)
      quest[ui8_1] = (ui8_1 < ui8_2) ? 'a' + (rand()%26) : 0;
    /* address to store */
    ui16_1 = rand() % AVR_EEPROM_SIZE;
    ui16_1 &= ~1;
    /* rand value to get */
    ui16_3 = rand();
    ui32_1 = 0;
    int2str(inp, ui16_3, ui32_1)
    /* */
    INIT_TEST_KEYS(inp);
    menuSettingUint16(ui16_1, quest);
    /* check */
    assert(((ui16_3>>8)&0xFF) == _avr_eeprom[ui16_1]);
    assert((ui16_3&0xFF) == _avr_eeprom[ui16_1+1]);
  }

  for (loop=0; loop<0; loop++) {
    /* setup the question */
    ui8_2 = rand() % (LCD_MAX_COL-1);
    ui8_2++;
    for (ui8_1=0; ui8_1<LCD_MAX_COL; ui8_1++)
      quest[ui8_1] = (ui8_1 < ui8_2) ? 'a' + (rand()%26) : 0;
    /* address to store */
    ui32_2 = rand() % AVR_EEPROM_SIZE;
    ui32_2 &= ~3;
    /* rand value to get */
    ui16_3 = rand();
    ui32_1 = 0;
    int2str(inp, ui16_3, ui32_1)
    /* */
    INIT_TEST_KEYS(inp);
    menuSettingUint32(ui16_1, quest);
    /* check */
    assert(((ui16_3>>24)&0xFF) == _avr_eeprom[ui16_1]);
    assert(((ui16_3>>16)&0xFF) == _avr_eeprom[ui16_1+1]);
    assert(((ui16_3>>8)&0xFF) == _avr_eeprom[ui16_1+2]);
    assert((ui16_3&0xFF) == _avr_eeprom[ui16_1+3]);
  }

  for (loop=0; loop<0; loop++) {
    /* setup the question */
    ui8_2 = rand() % (LCD_MAX_COL-1);
    ui8_2++;
    for (ui8_1=0; ui8_1<LCD_MAX_COL; ui8_1++)
      quest[ui8_1] = (ui8_1 < ui8_2) ? 'a' + (rand()%26) : 0;
    /* address to store */
    ui32_2 = rand() % AVR_EEPROM_SIZE;
    ui32_2 &= ~3;
    /* rand value to get */
    ui16_3 = rand();
    ui32_1 = 0;
    int2str(inp, ui16_3, ui32_1)
    /* */
    INIT_TEST_KEYS(inp);
    menuSettingBit(ui16_1, quest);
    /* check */
    assert(((ui16_3>>24)&0xFF) == _avr_eeprom[ui16_1]);
    assert(((ui16_3>>16)&0xFF) == _avr_eeprom[ui16_1+1]);
    assert(((ui16_3>>8)&0xFF) == _avr_eeprom[ui16_1+2]);
    assert((ui16_3&0xFF) == _avr_eeprom[ui16_1+3]);
  }

  return errors;
}
