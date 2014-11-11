#include <stdint.h>
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <util/twi.h>

#include "assert.h"
#include "billing.h"

/* Save, restore bill */
void
billingInit(void)
{
  assert(6 == sizeof(struct sale_info));
  assert(10 == sizeof(struct sale_item));
  assert(241 == SALE_SIZEOF);
  assert(SALE_SIZEOF <= BUFSS_SIZE);
}

void
billingStore(struct sale *sl, uint8_t ui8_5)
{
#if FF_ENABLE
  uint8_t ui8_3, ui8_4;
  uint16_t ui16_2;

  /* Save the bill to SD */
  UINT ret_val;
  memset(&FS, 0, sizeof(FS));
  memset(&Fil, 0, sizeof(Fil));
  //  change_sd(0); /* FIXME: */
  f_mount(&FS, "", 0);		/* Give a work area to the default drive */
  strncpy_P(bufSS, SD_BILLING_FILE, sizeof(SD_BILLING_FILE));
  bufSS[sizeof(SD_BILLING_FILE)] = 0;
  if (f_open(&Fil, bufSS, FA_READ|FA_WRITE|FA_CREATE_ALWAYS) == FR_OK) {	/* Create a file */
    do {
      /* Move to end of the file to append data */
      ui32_2 = f_size(&Fil);
      if (0 != ui32_2) {    /* file is old */
	assert( 2 == (ui32_2 %
		      (SALE_SIZEOF+((MAX_ITEMS_IN_BILL-1)*ITEM_SIZEOF)+2)) );
	/* If version doesn't match, escape... */
	f_lseek(&Fil, 0);
	f_read(&Fil, bufSS, 2, &ret_val);
	assert(2 == ret_val);
	ui16_2 = bufSS[0]; ui16_2 <<= 8; ui16_2 |= bufSS[1];
	if (GIT_HASH_CRC != ui16_2) {
	  LCD_ALERT(PSTR("Del old files"));
	  break;
	}
	/* */
	f_lseek(&Fil, ui32_2-2);
	f_read(&Fil, bufSS, 2, &ret_val);
	assert(2 == ret_val);
	ui16_2 = bufSS[0]; ui16_2 <<= 8; ui16_2 |= bufSS[1];
	f_lseek(&Fil, ui32_2-2);
      } else {     /* file already exists */
	/* new file, set it up ... */
	f_lseek(&Fil, 0);
	ui16_2 = 0;
	ui16_2 = _crc16_update(ui16_2, (GIT_HASH_CRC>>8)&0xFF);
	ui16_2 = _crc16_update(ui16_2, GIT_HASH_CRC&0xFF);
	bufSS[0] = GIT_HASH_CRC>>8;
	bufSS[1] = GIT_HASH_CRC&0xFF;
	f_write(&Fil, (void *)bufSS, 2, &ret_val);
	assert(2 == ret_val);
      }

      /* now save the data */
      f_write(&Fil, (void *)sl, SALE_SIZEOF-ITEM_SIZEOF, &ret_val);
      assert((SALE_SIZEOF-ITEM_SIZEOF) == ret_val);
      for (ui8_4=0; ui8_4<(SALE_SIZEOF-ITEM_SIZEOF); ui8_4++)
	ui16_2 = _crc16_update(ui16_2, ((uint8_t *)sl)[ui8_4]);
      /* save item */
      for (ui8_4=0; ui8_4<ui8_5; ui8_4++) {
	ee24xx_read_bytes(sl->items[ui8_4].ep_item_ptr, (void *)&(sl->it[0]), ITEM_SIZEOF);
	f_write(&Fil, &(sl->it[0]), ITEM_SIZEOF, &ret_val);
	for (ui8_3=0; ui8_3<ITEM_SIZEOF; ui8_3++)
	  ui16_2 = _crc16_update(ui16_2, ((uint8_t *)&(sl->it[0]))[ui8_3]);
	assert(ITEM_SIZEOF == ret_val);
      }
      memset((void *)&(sl->it[0]), 0, ITEM_SIZEOF);
      for (; ui8_4<MAX_ITEMS_IN_BILL; ui8_4++) {
	f_write(&Fil, &(sl->it[0]), ITEM_SIZEOF, &ret_val);
	assert(ITEM_SIZEOF == ret_val);
	for (ui8_3=0; ui8_3<ITEM_SIZEOF; ui8_3++)
	  ui16_2 = _crc16_update(ui16_2, 0);
      }
      /* */
      bufSS[0] = ui16_2>>8;
      bufSS[1] = ui16_2;
      f_write(&Fil, bufSS, 2, &ret_val);
      assert(2 == ret_val);
      /* */
      f_close(&Fil);
    } while (0);
  } else
    LCD_ALERT(PSTR("Can't save bill"));
  f_mount(NULL, "", 0);
#endif
}

void
billingDeleteAll()
{
}

void
billingIterStart()
{
}

void
billingIterNext(struct sale *sl)
{
  memset((void *)sl, sizeof(struct sale));
}
