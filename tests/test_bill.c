#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/crc16.h>
#include <avr/eeprom.h>

#include "ep_ds.h"
#include "version.h"
#include "assert.h"
#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#include "billing.h"
#include "i2c.h"
#include "uart.h"
#include "a1micro2mm.h"
#include "menu.h"
#include "main.h"

//assert(SPM_PAGESIZE == (1<<(FLASH_PAGE_SIZE_LOGN+1)));

#include "lcd.c"
//#include "kbd.c"
#include "ep_store.c"
#include "i2c.c"
#include "uart.c"
//#include "mmc_avr.c"
//#include "ff.c"
#include "a1micro2mm.c"
//#include "menu.c"
//#include "main.c"

uint8_t bufSS[BUFSS_SIZE];

void
eeprom_update_byte_NP(uint16_t addr, uint8_t *pstr, uint8_t size)
{
  uint8_t ui8_1;
  for (; size; size--) {
    ui8_1 = pgm_read_byte(pstr);
    if (0 == ui8_1) break;
    eeprom_update_byte(addr, ui8_1);
    addr++;
    pstr++;
  }
  for (; size; size--) {
    eeprom_update_byte(addr, ' ');
    addr++;
  }
}

void
menuPrnBill(struct sale *sl)
{
  uint8_t ui8_1, ui8_2, ui8_3;
  uint8_t prnBuf[256];

  /* Shop name */
  PRINTER_FONT_ENLARGE(2);
  PRINTER_JUSTIFY(PRINTER_JCENTER);
  for (ui8_2=ui8_1=0; ui8_1<SHOP_NAME_SZ_MAX; ) {
    if (ui8_2 && (0 == (ui8_2%(PRINTER_MAX_CHARS_ON_LINE>>1)))) {
      PRINTER_PRINT('\n');
      ui8_2 = 0;
    } else {
      ui8_3 = eeprom_read_byte((void *)(offsetof(struct ep_store_layout, shop_name)+ui8_1));
      PRINTER_PRINT(ui8_3);
      ui8_1++;
    }
    ui8_2++;
  }
  PRINTER_FONT_ENLARGE(1);

  /* Header */
  for (ui8_1=0; ui8_1<HEADER_SZ_MAX; ui8_1++) {
    ui8_3 = eeprom_read_byte((void *)(offsetof(struct ep_store_layout, b_head)+ui8_1));
    PRINTER_PRINT(ui8_3);
  }

  PRINTER_JUSTIFY(PRINTER_JLEFT);

  /* Caption, user, Date */
  for (ui8_1=0; ui8_1<EPS_CAPTION_SZ_MAX; ui8_1++) {
    ui8_3 = eeprom_read_byte((void *)(offsetof(struct ep_store_layout, caption)+ui8_1));
    if ('\n' != ui8_3)
      PRINTER_PRINT(ui8_3);
  }
  PRINTER_PRINT(' '); PRINTER_PRINT(' ');
  PRINTER_PRINT('u'); PRINTER_PRINT('s'); PRINTER_PRINT('e');
  PRINTER_PRINT('r'); PRINTER_PRINT(':');
  for (ui8_1=0; ui8_1<EPS_MAX_UNAME; ui8_1++) {
    ui8_3 = sl->info.user[ui8_1];
    assert ('\n' != ui8_3);
    PRINTER_PRINT(ui8_3);
  }
  PRINTER_PRINT(' '); PRINTER_PRINT(' ');
  PRINTER_SPRINTF(prnBuf, "%2d", 1+sl->info.date_dd); PRINTER_PRINT('/');
  PRINTER_SPRINTF(prnBuf, "%2d", 1+sl->info.date_mm); PRINTER_PRINT('/');
  PRINTER_SPRINTF(prnBuf, "%4d", 1980+sl->info.date_yy); PRINTER_PRINT(' ');
  PRINTER_SPRINTF(prnBuf, "%2d", sl->info.time_hh); PRINTER_PRINT(':');
  PRINTER_SPRINTF(prnBuf, "%02d", sl->info.time_mm); PRINTER_PRINT(':');
  PRINTER_SPRINTF(prnBuf, "%02d", sl->info.time_ss); PRINTER_PRINT('\n');

  /* Items */
  for (ui8_1=0; ui8_1<sl->info.n_items; ui8_1++) {
    if (EEPROM_MAX_ADDRESS != sl->items[ui8_1].ep_item_ptr) {
      ee24xx_write_bytes(sl->items[ui8_1].ep_item_ptr,
			 (void *)&(sl->it[0]), ITEM_SIZEOF);
      ui8_2 = 0;
    } else {
      ui8_2 = ui8_1;
    }
    PRINTER_SPRINTF(prnBuf, "%2d. ", ui8_1);
    for (ui8_3=0; ui8_3<ITEM_NAME_BYTEL; ui8_3++)
      PRINTER_PRINT(sl->it[0].name[ui8_3]);
    PRINTER_SPRINTF(prnBuf, " %4d", sl->items[ui8_3].cost);
    PRINTER_SPRINTF(prnBuf, "(-%4d)", sl->items[ui8_3].discount);
    PRINTER_SPRINTF(prnBuf, " %4d", sl->items[ui8_3].quantity);
    PRINTER_SPRINTF(prnBuf, " %6d\n", sl->items[ui8_3].cost * sl->items[ui8_3].quantity);
  }

  /* Total */
  PRINTER_SPRINTF(prnBuf, "Total Discount : %.2f\n", sl->t_discount);
  PRINTER_SPRINTF(prnBuf, "Total VAT      : %.2f\n", sl->t_vat);
  PRINTER_SPRINTF(prnBuf, "Total Serv Tax : %.2f\n", sl->t_stax);
  PRINTER_SPRINTF(prnBuf, "Bill Total (Rs): %.2f\n", sl->total);

  /* Footer */
  ui8_2 = 0;
  for (ui8_1=0; ui8_1<FOOTER_SZ_MAX; ui8_1++) {
    ui8_3 = eeprom_read_byte((void *)(offsetof(struct ep_store_layout, b_foot)+ui8_1));
    ui8_2 = ('\n' == ui8_3) ? 0 :
      ( (PRINTER_MAX_CHARS_ON_LINE == ui8_2) ? 0 : ui8_2+1 );
    if (0 == ui8_2) {
      PRINTER_PRINT('\n');
    }
    if ('\n' != ui8_3)
      PRINTER_PRINT(ui8_3);
  }
}

void
menuFactorySettings(uint8_t mode)
{
  uint8_t ui8_1;
  uint16_t ui16_1;

  /* Mark all items as deleted : (0==id) */
  for (ui8_1=0; ui8_1<ITEM_SIZEOF; ui8_1++)
    bufSS[ui8_1] = 0;
  for (ui16_1=0; ui16_1 < ITEM_MAX_ADDR;
       ui16_1 += (ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2) ) {
    ee24xx_write_bytes(ui16_1+(offsetof(struct item, id)>>EEPROM_MAX_DEVICES_LOGN2),
		       bufSS+offsetof(struct item, id), EEPROM_MAX_DEVICES_LOGN2);
  }

  /* */
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, shop_name),
			PSTR("Sri Ganapathy Medicals"), SHOP_NAME_SZ_MAX);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, unused_serial_no),
			PSTR("ABCDEFGHIJKL"), SERIAL_NO_MAX);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, b_head),
			PSTR("12 Agraharam street, New Tippasandara,\n Bangalore - 52\n TIN:299007249"), HEADER_SZ_MAX);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, b_foot),
			PSTR("Welcome & come back..."), FOOTER_SZ_MAX);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, currency),
			PSTR("Rupees"), EPS_WORD_LEN);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, b_pfix),
			PSTR("A000"), EPS_WORD_LEN);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, caption),
			PSTR("Invoice"), EPS_CAPTION_SZ_MAX);
}

int
main(void)
{
  uint8_t ui1;

  LCD_init();

  uartInit();
  uartSelect(0);

  /* Enable Int0 on falling edge */
//  GICR = 1<<INT0;
//  MCUCR |= 1<<ISC01 | 0<<ISC00;

  /* Enable Global Interrupts */
  sei();

  PORTD |= 0x10;
  LCD_WR_LINE_NP(0, 0, PSTR("Bill Sample: "), 13);
  LCD_refresh();
  _delay_ms(1000);

  struct sale *sl = (void *) bufSS;
  sl->info.n_items = 1;
  sl->info.prop    = 0;
  strncpy_P(sl->info.user, PSTR("naren"), 5);
  sl->info.date_yy = 44;
  sl->info.date_mm = 9;
  sl->info.date_dd = 4;
  sl->items[0].ep_item_ptr = 0;
  sl->items[0].quantity = 5;
  sl->items[0].cost = 55;
  sl->items[0].discount = 13;
  sl->items[0].has_common_discount = 0;
  sl->items[0].has_vat = 0;
  sl->items[0].has_serv_tax = 0;
  sl->items[0].vat_sel = 0;
  sl->t_stax = 233;
  sl->t_discount = 144;
  sl->t_vat = 33;
  sl->total = 16734;
  sl->it[0].id = 23;
  sl->it[0].cost = 5500;
  sl->it[0].discount = 1155;
  strncpy_P(sl->it[0].name, PSTR("milk n cream"), 12);
  strncpy_P(sl->it[0].prod_code, PSTR("98798sdfaiuy988a"), 16);
  sl->it[0].has_serv_tax = 1;
  sl->it[0].has_common_discount = 0;
  sl->it[0].has_weighing_mc = 1;
  sl->it[0].vat_sel = 0;
  sl->it[0].name_in_unicode = 0;
  sl->it[0].has_vat = 1;
  sl->it[0].is_disabled = 0;

  menuPrnBill(sl);

  LCD_WR_LINE_NP(1, 0, PSTR("Printed"), 7);
  LCD_refresh();

  while (1) {}

  return 0;
}
