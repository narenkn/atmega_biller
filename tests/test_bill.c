#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
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
#include "main.h"
#include "menu.h"

//assert(SPM_PAGESIZE == (1<<(FLASH_PAGE_SIZE_LOGN+1)));

#include "lcd.c"
//#include "kbd.c"
#include "ep_store.c"
//#include "i2c.c"
#include "uart.c"
//#include "mmc_avr.c"
//#include "ff.c"
#include "a1micro2mm.c"
//#include "menu.c"
//#include "main.c"

date_t i2c_date = {1, 10, 2016};
s_time_t i2c_time;
#undef  timerDateGet
#undef  timerTimeGet
#define timerDateGet(d) d = i2c_date
#define timerTimeGet(t) t = i2c_time

#define menuMemset(S, c, N) do {			\
    for (uint16_t _ui16_1=0; _ui16_1<N; _ui16_1++) {	\
      ((uint8_t *)S)[_ui16_1] = c;			\
    }							\
  } while (0)

uint8_t bufSS[BUFSS_SIZE];

void
eeprom_update_byte_NP(uint16_t addr, const char *pstr, uint8_t size)
{
  uint8_t ui8_1;
  for (; size; size--) {
    ui8_1 = pgm_read_byte(pstr);
    if (0 == ui8_1) break;
    eeprom_update_byte((uint8_t *)addr, ui8_1);
    addr++;
    pstr++;
  }
  for (; size; size--) {
    eeprom_update_byte((uint8_t *)addr, ' ');
    addr++;
  }
}

void
menuPrnBillEE24xxHelper(uint16_t item_addr, struct item *it, uint16_t it_index)
{
#if 0
  ee24xx_read_bytes(item_addr, (void *)it, ITEM_SIZEOF);
#endif
}

static uint8_t
menuPrnD(uint32_t var, uint8_t charsPrinted)
{
  if (var>9)
    charsPrinted = menuPrnD(var/10, charsPrinted);
  if (charsPrinted & 0x80) PRINTER_PRINT('0'+(var%10));
  return charsPrinted + 1;
}

static uint8_t
menuPrnF(uint32_t var, uint8_t charsPrinted)
{
  charsPrinted = menuPrnD(var/100, charsPrinted);
  if (charsPrinted & 0x80) PRINTER_PRINT('.');
  charsPrinted++;
  var %= 100;
  if (var < 10) {
    if (charsPrinted & 0x80) PRINTER_PRINT('0');
    charsPrinted++;
  }
  return menuPrnD(var%100, charsPrinted);
}

static bool firstTimePrnHeader = true;
static void
menuPrnHeader()
{
  uint8_t ui8_1, ui8_2, ui8_3;

  /* */
  if (!firstTimePrnHeader) {
    return;
  }

  /* Shop name */
  PRINTER_FONT_ENLARGE(PRINTER_FONT_2);
  PRINTER_JUSTIFY(PRINTER_JCENTER);
  for (ui8_2=ui8_1=0; ui8_1<SHOP_NAME_SZ_MAX; ui8_2++, ui8_1++) {
#if 0 /* not required */
    if (ui8_2 >= (PRINTER_FONTA_CHARSINLINE>>1)) {
      PRINTER_PRINT_NL;
      ui8_2 = 0;
    }
#endif
    ui8_3 = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, ShopName)+ui8_1));
    if (0 == ui8_3) break;
    PRINTER_PRINT(ui8_3);
  }
  PRINTER_FONT_ENLARGE(PRINTER_FONT_1);
  PRINTER_MODE_SEL(PRINTER_CHAR_FONTB);

  /* Header */
  for (ui8_1=0; ui8_1<HEADER_SZ_MAX; ui8_1++) {
    ui8_3 = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, BillHeader)+ui8_1));
    PRINTER_PRINT(ui8_3);
  }
  PRINTER_PRINT_NL;

  /* */
  firstTimePrnHeader = false;
}

static void
menuPrnFooter()
{
  uint8_t ui8_1, ui8_2, ui8_3;

  /* Footer */
  ui8_2 = 0;
  for (ui8_1=0; ui8_1<FOOTER_SZ_MAX; ui8_1++) {
    ui8_3 = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, BillFooter)+ui8_1));
    ui8_2 = ('\n' == ui8_3) ? 0 :
      ( (PRINTER_FONTA_CHARSINLINE == ui8_2) ? 0 : ui8_2+1 );
    if (0 == ui8_2) {
      PRINTER_PRINT_NL;
    }
    if ('\n' != ui8_3)
      PRINTER_PRINT(ui8_3);
  }
  PRINTER_PRINT_NL;
}

void
menuPrnBill(struct sale *sl, menuPrnBillItemHelper nitem)
{
#if MENU_PRINTER_ENABLE
  uint8_t ui8_1, ui8_3;

  menuPrnHeader();

  /* Caption, Date */
  PRINTER_FONT_ENLARGE(PRINTER_FONT_2);
  PRINTER_JUSTIFY(PRINTER_JCENTER);
  for (ui8_1=0; ui8_1<EPS_CAPTION_SZ_MAX; ui8_1++) {
    ui8_3 = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, Caption)+ui8_1));
    if ('\n' != ui8_3)
      PRINTER_PRINT(ui8_3);
  }
  PRINTER_PRINT_NL;

  /* */
  PRINTER_FONT_ENLARGE(PRINTER_FONT_1);
  PRINTER_JUSTIFY(PRINTER_JLEFT);
  {
    date_t date;
    timerDateGet(date);
    PRINTER_PSTR(PSTR(" Printed:"));
    PRINTER_PRINT_D(date.day); PRINTER_PRINT('/');
    PRINTER_PRINT_D(date.month); PRINTER_PRINT('/');
    PRINTER_PRINT_D(date.year);
  }
  PRINTER_PSTR(PSTR("  by:"));
  for (ui8_1=0; ui8_1<EPS_MAX_UNAME; ui8_1++) {
    ui8_3 = sl->info.user[ui8_1];
    PRINTER_PRINT(ui8_3);
  }
  PRINTER_PRINT_NL;

  /* */
  PRINTER_PSTR(PSTR(" Bill #"));
  PRINTER_PRINT_D(sl->info.id);
  PRINTER_PRINT(' '); PRINTER_PRINT('@');
  PRINTER_PRINT_D(sl->info.date_dd); PRINTER_PRINT('/');
  PRINTER_PRINT_D(sl->info.date_mm); PRINTER_PRINT('/');
  PRINTER_PRINT_D(sl->info.date_yy); PRINTER_PRINT(' ');
  PRINTER_PRINT_D(sl->info.time_hh); PRINTER_PRINT(':');
  PRINTER_PRINT_D(sl->info.time_mm); PRINTER_PRINT(':');
  PRINTER_PRINT_D(sl->info.time_ss); PRINTER_PRINT_NL;
  PRINTER_MODE_SEL(PRINTER_CHAR_FONTB);
  PRINTER_PSTR(PSTR("Sl. Item              Qty   Price  Sub-Tot\n"));
  PRINTER_PSTR(PSTR("------------------------------------------\n"));

  /* Items */
  for (ui8_1=0; ui8_1<sl->info.n_items; ui8_1++) {
    if (EEPROM_MAX_ADDRESS != sl->items[ui8_1].ep_item_ptr) {
      nitem(sl->items[ui8_1].ep_item_ptr, &(sl->it[0]), ui8_1);
    }
    ui8_3 = PRINTER_PRINT_D_N((ui8_1+1));
    for (;ui8_3<2; ui8_3++) {
      PRINTER_PRINT(' ');
    }
    PRINTER_PRINT_D((ui8_1+1));
    PRINTER_PRINT('.');
    for (ui8_3=0; ui8_3<((ITEM_NAME_BYTEL<16)?ITEM_NAME_BYTEL:16); ui8_3++) {
      PRINTER_PRINT(sl->it[0].name[ui8_3]);
    }
    PRINTER_PRINT(' ');
    ui8_3 = PRINTER_PRINT_F_N(sl->items[ui8_1].quantity/10);
    for (;ui8_3<6; ui8_3++) {
      PRINTER_PRINT(' ');
    }
    PRINTER_PRINT_F(sl->items[ui8_1].quantity/10); PRINTER_PRINT(' ');
    ui8_3 = PRINTER_PRINT_F_N(sl->items[ui8_1].cost);
    for (;ui8_3<7; ui8_3++) {
      PRINTER_PRINT(' ');
    }
    PRINTER_PRINT_F(sl->items[ui8_1].cost); PRINTER_PRINT(' ');
    uint32_t total = (sl->items[ui8_1].cost-sl->items[ui8_1].discount)/100*sl->items[ui8_1].quantity/1000;
#if 0 /* FIXME */
    uint32_t t_tax;
    t_tax = total * 
    if (! sl->items[ui8_1].is_reverse_tax) {
      total 
    }
#endif
    ui8_3 = PRINTER_PRINT_F_N(total);
    for (;ui8_3<7; ui8_3++) {
      PRINTER_PRINT(' ');
    }
    PRINTER_PRINT_F(total);
    PRINTER_PRINT_NL;
  }

  /* Total */
  PRINTER_PSTR(PSTR("------------------------------------------\n"));
/*PRINTER_PSTR(PSTR("Sl. Item             Qty   Price  SubTotal\n"));*/
  PRINTER_PSTR(PSTR("             Total Discount: "));
  ui8_3 = PRINTER_PRINT_F_N(sl->t_discount);
  for (;ui8_3<12; ui8_3++) {
    PRINTER_PRINT(' ');
  }
  PRINTER_PRINT_F(sl->t_discount);
  PRINTER_PRINT_NL;
  PRINTER_PSTR(PSTR("                  Total Tax: "));
  ui8_3 = PRINTER_PRINT_F_N(sl->t_tax1+sl->t_tax2+sl->t_tax3+sl->t_vat);
  for (;ui8_3<12; ui8_3++) {
    PRINTER_PRINT(' ');
  }
  PRINTER_PRINT_F(sl->t_tax1+sl->t_tax2+sl->t_tax3+sl->t_vat);
  PRINTER_PRINT_NL;
  PRINTER_PSTR(PSTR("------------------------------------------\n"));
  PRINTER_PSTR(PSTR("                      Total: "));
  ui8_3 = PRINTER_PRINT_F_N(sl->total);
  for (;ui8_3<12; ui8_3++) {
    PRINTER_PRINT(' ');
  }
  PRINTER_PRINT_F(sl->total);
  PRINTER_PRINT_NL;
  PRINTER_MODE_SEL(PRINTER_CHAR_FONTA);

  menuPrnFooter();

  firstTimePrnHeader = true;
  menuPrnHeader();
#endif
}

uint8_t
menuFactorySettings(uint8_t mode)
{
  uint8_t ui8_1;
  uint16_t ui16_1;

#if 0
  /* Mark all items as deleted : (0==id) */
  for (ui8_1=0; ui8_1<ITEM_SIZEOF; ui8_1++)
    bufSS[ui8_1] = 0;
  for (ui16_1=0; ui16_1 < ITEM_MAX_ADDR;
       ui16_1 += (ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2) ) {
    ee24xx_write_bytes(ui16_1+(offsetof(struct item, id)>>EEPROM_MAX_DEVICES_LOGN2),
		       bufSS+offsetof(struct item, id), EEPROM_MAX_DEVICES_LOGN2);
  }
#endif

  /* */
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, ShopName),
			PSTR("Sri Ganapathy Medicals"), SHOP_NAME_SZ_MAX);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, unused_serial_no),
			PSTR("ABCDEFGHIJKL"), SERIAL_NO_MAX);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, BillHeader),
			PSTR("12 Agraharam street, New Tippasandara,\n Bangalore - 52\n TIN:299007249"), HEADER_SZ_MAX);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, BillFooter),
			PSTR("Welcome & come back..."), FOOTER_SZ_MAX);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, Currency),
			PSTR("Rupees"), EPS_WORD_LEN);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, BillPrefix),
			PSTR("A000"), EPS_WORD_LEN);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, Caption),
			PSTR("Invoice"), EPS_CAPTION_SZ_MAX);

  return 0;
}

int
main(void)
{
  LCD_init();
  uartInit();
  printerInit();

  /* Enable Int0 on falling edge */
//  GICR = 1<<INT0;
//  MCUCR |= 1<<ISC01 | 0<<ISC00;

  /* Enable Global Interrupts */
//  sei();

  LCD_bl_on;
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("Bill Sample: "));
  _delay_ms(1000);
  BUZZER_ON;
  _delay_ms(50);
  BUZZER_OFF;

  menuMemset((void *)bufSS, 0, BUFSS_SIZE);

  struct sale *sl = (void *) bufSS;
  sl->info.n_items = 1;
  strncpy_P((char *)sl->info.user, PSTR("naren       "), EPS_MAX_UNAME);
  sl->info.date_yy = 17;
  sl->info.date_mm = 9;
  sl->info.date_dd = 4;
  sl->items[0].ep_item_ptr = 0;
  sl->items[0].quantity = 5;
  sl->items[0].cost = 55;
  sl->items[0].discount = 13;
  sl->items[0].has_common_discount = 0;
  sl->items[0].has_vat = 0;
  sl->items[0].has_tax1 = 0;
  sl->items[0].has_tax2 = 0;
  sl->items[0].has_tax3 = 0;
  sl->t_tax1 = 233;
  sl->t_tax2 = 234;
  sl->t_tax3 = 235;
  sl->t_discount = 144;
  sl->total = 16734;
  sl->t_cash_pay = 12345;
  sl->it[0].id = 23;
  sl->it[0].cost = 5500;
  sl->it[0].discount = 1155;
  strncpy_P((char *)sl->it[0].name, PSTR("milk n cream"), 12);
  strncpy_P((char *)sl->it[0].prod_code, PSTR("98798sdfaiuy988a"), 16);
  sl->it[0].has_tax1 = 1;
  sl->it[0].has_tax2 = 1;
  sl->it[0].has_tax3 = 1;
  sl->it[0].has_common_discount = 0;
  sl->it[0].has_weighing_mc = 1;
  sl->it[0].is_reverse_tax = 0;

  menuPrnBill(sl, menuPrnBillEE24xxHelper);

  LCD_WR_P(PSTR("Printed"));

  while (1) {}

  return 0;
}
