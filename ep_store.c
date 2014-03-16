#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#include "ep_store.h"

void
ep_store_init(void)
{
  uint16_t eeprom_crc, ui1;
  uint8_t  ui2;
  uint8_t eeprom_idx;
  uint8_t ymd[3];

  /* check CRC and init everything if screwed */
  CRC16_Init();
  for (ui1=0; ui1<(uint16_t)&(EEPROM_DATA.eeprom_sig[0]); ui1++) {
    EEPROM_STORE_READ(ui1, &ui2, sizeof(uint8_t));
    CRC16_Update(ui2);
  }
  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.eeprom_idx), (uint8_t *)&eeprom_idx, sizeof(uint8_t));
  eeprom_crc = CRC16_High; eeprom_crc <<= 8; eeprom_crc |= CRC16_Low;
  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.eeprom_sig[eeprom_idx]), (uint8_t *)&ui1, sizeof(uint16_t));
  if ((eeprom_idx < EEPROM_DYNARR_MAX) && (eeprom_crc == ui1)) {
    /* */
    EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.bill_id[eeprom_idx]), (uint8_t *)&ui1, sizeof(uint16_t));
    eeprom_idx++;
    eeprom_idx %= EEPROM_DYNARR_MAX;
    EEPROM_STORE_WRITE_NoSig((uint16_t)&(EEPROM_DATA.bill_id[eeprom_idx]), (uint8_t *)&ui1, sizeof(uint16_t));

    /* Finally write the signature */
    EEPROM_STORE_WRITE_Sig;

    return;
  }

  /* Warn of data loss */
  LCD_CLRSCR;
  LCD_WR_LINE(0, 0, "Data Fail, Loss");
  LCD_WR_LINE(1, 0, "Serv if Repeat");

  /* clear all bytes of EEPROM */
  ui2 = 0;
  for (ui1=0; ui1<sizeof(struct ep_store_layout); ui1++) {
    EEPROM_STORE_WRITE_NoSig(ui1, (uint8_t *)&ui2, sizeof(uint8_t));
  }

  /* */
  ui1 = 0;
  EEPROM_STORE_WRITE_NoSig((uint16_t)&(EEPROM_DATA.item_last_modified), (uint8_t *)&ui1, sizeof(uint16_t));
  for (ui1=FLASH_ITEM_START; ui1<FLASH_ITEM_END; ui1+=FLASH_SECTOR_SIZE) {
    FlashEraseSector(ui1);
  }

  /* for data location, init and clear the first sector */
  ui1 = FLASH_DATA_START;
  EEPROM_STORE_WRITE_NoSig((uint16_t)&(EEPROM_DATA.sale_start), (uint8_t *)&ui1, sizeof(uint16_t));
  EEPROM_STORE_WRITE_NoSig((uint16_t)&(EEPROM_DATA.sale_end), (uint8_t *)&ui1, sizeof(uint16_t));
  FlashEraseSector(FLASH_DATA_START);

  /* */
  timerDateGet(ymd);
  ui2 = PREV_MONTH(ymd[1]);
  EEPROM_STORE_WRITE_NoSig((uint16_t)&(EEPROM_DATA.sale_date_old_ptr_month), (uint8_t *)&ui2, sizeof(uint8_t));

  /* */
  EEPROM_STORE_WRITE_NoSig((uint16_t)&(EEPROM_DATA.prn_header[0]), (uint8_t *)"Example Hotel", sizeof(uint8_t));

  /* Recompute hash */
  eeprom_idx = 0;
  EEPROM_STORE_WRITE_NoSig((uint16_t)&(EEPROM_DATA.eeprom_idx), (uint8_t *)&eeprom_idx, sizeof(uint8_t));
  EEPROM_STORE_WRITE_Sig;

  /* */
  LCD_CLRSCR;
}

/* Organise the pointers as
   At start
     sale_date_ptr[48]==0, sale_date_old_ptr==0, sale_date_old_ptr_month==0
     set sale_date_old_ptr_month as prev month, but no data would exist
   After storing 12 months.. when we overflow, keep 13th month data
     At any point, valid data is from sale_date_old_ptr_month onwards.
 */
void
sale_index_it(sale_info *si, uint16_t ptr)
{
  uint8_t date, month, ui2, ui3;
  uint16_t p1, ui1;

  /* check values */
  date = si->date_dd;
  month = si->date_mm;
  assert ((date >= 1) && (date <= 31));
  assert ((month >= 1) && (month <= 12));

  /* if month is same, we have rolled back*/
  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.sale_date_old_ptr_month), (uint8_t *)&ui3, sizeof(uint8_t));
  if (PREV_MONTH(month) == ui3) {
    /* first release data of prev 13th month */
    flash_sale_delete_month(ui3);
    /* transfer 12th month to 13th month */
    ui3 = (month-1)*4;
    for (ui2=0; ui2<4; ui2++) {
      /* then store 12th month data to 13th month */
      EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.sale_date_ptr[ui3+ui2]), (uint8_t *)&p1, sizeof(uint16_t));
      EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.sale_date_old_ptr[ui2]), (uint8_t *)&p1, sizeof(uint16_t));
      p1 = 0;
      EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.sale_date_ptr[ui3+ui2]), (uint8_t *)&p1, sizeof(uint16_t));
    }
    EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.sale_date_old_ptr_month), (uint8_t *)&month, sizeof(uint8_t));
  }

  /* Check if required to be indexed */
  ui2 =  (month-1)*4;
  if (date <= 7) {}
  else if (date <= 14) ui2 ++;
  else if (date <= 21) ui2 += 2;
  else ui2 += 3;
  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.sale_date_ptr[ui2]), (uint8_t *)&p1, sizeof(uint16_t));
  if (0 == p1) { /* not indexed yet */
    EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.sale_date_ptr[ui2]), (uint8_t *)ptr, sizeof(uint16_t));
  }

  /* update id */
  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.date_month), (uint8_t *)&p1, sizeof(uint16_t));
  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.eeprom_idx), (uint8_t *)&ui2, sizeof(uint8_t));
  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.bill_id[ui2]), (uint8_t *)&ui1, sizeof(uint16_t));
  if ((date != ((p1>>8)&0xFF)) || (month != (p1&0xFF))) {
    p1 = date; p1<<=8; p1|=month;
    EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.date_month), (uint8_t *)&p1, sizeof(uint16_t));
    ui1 = 1;
  } else ui1++;
  EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.bill_id[ui2]), (uint8_t *)&ui1, sizeof(uint16_t));
}
