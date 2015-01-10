#define TEST_KEY_ARR_SIZE    1024

#include "test_common.c"

/* test setting set routines */

/* place to store items */
struct item all_items[ITEM_MAX];

int
main()
{
  uint32_t errors;
  uint8_t  ui8_1, ui8_2, ui8_3, ui8_4, ui8_5;
  uint32_t loop, ui32_1, ui32_2;
  uint16_t ui16_1, ui16_2, ui16_3;
  uint8_t quest[LCD_MAX_COL];

  assert_init();
  KbdInit();

  for (loop=0; loop<1000; loop++) {
    /* setup the question */
    ui8_2 = rand() % (LCD_MAX_COL-1);
    ui8_2++;
    for (ui8_1=0; ui8_1<LCD_MAX_COL; ui8_1++)
      quest[ui8_1] = (ui8_1 < ui8_2) ? 'a' + (rand()%26) : 0;
    /* size of string to get */
    ui16_2 = rand() % BUFSS_SIZE;
    ui16_2++;
    /* address to store */
    ui16_1 = rand() % AVR_EEPROM_SIZE;
    if ((ui16_1+ui16_2) >= AVR_EEPROM_SIZE)
      ui16_1 -= ui16_2;
    /* setup the string */
    for (ui16_3=0; ui16_3<ui16_2; ui16_3++) {
      inp[ui16_3] = ' ' + rand() % ('~'-' ');
    }
    inp[ui16_2] = 0;
    INIT_TEST_KEYS(inp);
    menuSettingString(ui16_1, quest, ui16_2);
    /* check question */
    for (ui8_3=0; ui8_3<LCD_MAX_COL; ui8_3++)
      quest[ui8_3] = ((ui8_3<MENU_PROMPT_LEN)&&(0 == quest[ui8_3])) ? ' ' : (ui8_3<MENU_PROMPT_LEN) ? quest[ui8_3]: (ui8_3==(MENU_PROMPT_LEN+1)) ? '?' : ' ';
    assert(0 == strncmp(quest, lcd_buf[0], LCD_MAX_COL));
    /* check stored value */
    assert(0 == strncmp(inp, _avr_eeprom+ui16_1, ui16_3));
  }

  for (loop=0; loop<1000; loop++) {
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
    int2str(inp, ui8_1, &ui32_1);
    /* */
    INIT_TEST_KEYS(inp);
    menuSettingUint8(ui16_1, quest);
    /* check */
    for (ui8_3=0; ui8_3<LCD_MAX_COL; ui8_3++)
      quest[ui8_3] = ((ui8_3<MENU_PROMPT_LEN)&&(0 == quest[ui8_3])) ? ' ' : (ui8_3<MENU_PROMPT_LEN) ? quest[ui8_3]: (ui8_3==(MENU_PROMPT_LEN+1)) ? '?' : ' ';
    assert(0 == strncmp(quest, lcd_buf[0], LCD_MAX_COL));
    //printf("quest:'%s' lcd_buf[0]:'%s'\n", quest, lcd_buf[0]);
    //printf("addr:%d ui8_1:%x eeprom:%x\n", ui16_1, ui8_1, _avr_eeprom[ui16_1]);
    assert(ui8_1 == _avr_eeprom[ui16_1]);
  }

  for (loop=0; loop<1000; loop++) {
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
    int2str(inp, ui16_3, &ui32_1);
    /* */
    INIT_TEST_KEYS(inp);
    menuSettingUint16(ui16_1, quest);
    /* check */
    for (ui8_3=0; ui8_3<LCD_MAX_COL; ui8_3++)
      quest[ui8_3] = ((ui8_3<MENU_PROMPT_LEN)&&(0 == quest[ui8_3])) ? ' ' : (ui8_3<MENU_PROMPT_LEN) ? quest[ui8_3]: (ui8_3==(MENU_PROMPT_LEN+1)) ? '?' : ' ';
    assert(0 == strncmp(quest, lcd_buf[0], LCD_MAX_COL));
    assert(((ui16_3>>8)&0xFF) == _avr_eeprom[ui16_1]);
    assert((ui16_3&0xFF) == _avr_eeprom[ui16_1+1]);
  }

  for (loop=0; loop<1000; loop++) {
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
    int2str(inp, ui16_3, &ui32_1);
    /* */
    INIT_TEST_KEYS(inp);
    menuSettingUint32(ui32_2, quest);
    /* check */
    for (ui8_3=0; ui8_3<LCD_MAX_COL; ui8_3++)
      quest[ui8_3] = ((ui8_3<MENU_PROMPT_LEN)&&(0 == quest[ui8_3])) ? ' ' : (ui8_3<MENU_PROMPT_LEN) ? quest[ui8_3]: (ui8_3==(MENU_PROMPT_LEN+1)) ? '?' : ' ';
    assert(0 == strncmp(quest, lcd_buf[0], LCD_MAX_COL));
    assert(((ui16_3>>24)&0xFF) == _avr_eeprom[ui32_2+0]);
    assert(((ui16_3>>16)&0xFF) == _avr_eeprom[ui32_2+1]);
    assert(((ui16_3>>8)&0xFF) == _avr_eeprom[ui32_2+2]);
    assert((ui16_3&0xFF) == _avr_eeprom[ui32_2+3]);
  }

  for (loop=0; loop<1000; loop++) {
    /* setup the question */
    ui8_2 = rand() % (LCD_MAX_COL-1);
    ui8_2++;
    for (ui8_1=0; ui8_1<LCD_MAX_COL; ui8_1++)
      quest[ui8_1] = (ui8_1 < ui8_2) ? 'a' + (rand()%26) : 0;
    /* address to store */
    ui32_2 = rand() % AVR_EEPROM_SIZE;
    ui32_2 &= ~3;
    /* bit & bit-len */
    ui8_4 = rand() % 8;
    ui8_5 = rand() % (8-ui8_4); ui8_5++;
    /* rand value to get */
    ui16_3 = rand();
    ui32_1 = 0;
    int2str(inp, ui16_3, &ui32_1);
    /* */
    INIT_TEST_KEYS(inp);
    menuSettingBit(ui32_2, quest, ui8_5, ui8_4);
    /* check */
    for (ui8_3=0; ui8_3<LCD_MAX_COL; ui8_3++)
      quest[ui8_3] = ((ui8_3<MENU_PROMPT_LEN)&&(0 == quest[ui8_3])) ? ' ' : (ui8_3<MENU_PROMPT_LEN) ? quest[ui8_3]: (ui8_3==(MENU_PROMPT_LEN+1)) ? '?' : ' ';
    assert(0 == strncmp(quest, lcd_buf[0], LCD_MAX_COL));
    assert( (ui16_3 & ((1<<ui8_5)-1)) ==
	    ((_avr_eeprom[ui32_2]>>ui8_4) & ((1<<ui8_5)-1)) );
    //printf("val:%x\n", (ui16_3 & ((1<<ui8_5)-1)));
  }

  return errors;
}
