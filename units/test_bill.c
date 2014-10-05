#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "ep_ds.h"
#include "version.h"
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
#include "kbd.c"
#include "ep_store.c"
#include "i2c.c"
#include "uart.c"
#include "ff.c"
#include "a1micro2mm.c"
#include "menu.c"
#include "main.c"

#define LCD_WriteDirect(loc, str, len) do {	\
  uint8_t _ui1;					\
  LCD_cmd(loc);					\
  for (_ui1=0; _ui1<len; _ui1++) {		\
    LCD_wrchar( ((char *)str)[_ui1] );		\
  }						\
} while (0)

#define LCD_uint8x(ch) {			\
  uint8_t ui2_a = (ch>>4) & 0xF;		\
  ui2_a = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;	\
  LCD_wrchar(ui2_a);				\
  ui2_a = ch & 0xF;				\
  ui2_a = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;	\
  LCD_wrchar(ui2_a);				\
}

int
main(void)
{
  uint8_t ui1;

  LCD_init();

  uartInit();
  uartSelect(0);

  /* Set pin mode & enable pullup */
  DDRD &= ~((1<<PD2)|(1<<PD3));
//  PORTD |= (1<<PD2) | (1<<PD3);
  DDRD |= 0x10;
  PORTD |= 2<<5;

  /* Enable Int0 on falling edge */
  GICR = 1<<INT0;
  MCUCR |= 1<<ISC01 | 0<<ISC00;

  /* Enable Global Interrupts */
  sei();

  PORTD |= 0x10;
  LCD_WriteDirect(LCD_CMD_CUR_10, "Bill Sample: ", 13);
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

  return 0;
}
