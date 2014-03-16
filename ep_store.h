#ifndef EP_STORE_H
#define EP_STORE_H

#define HEADER_MAX_SZ       54
#define SHOP_NAME_SZ        16
#define FOOTER_MAX_SZ       24
#define MOD_HIS_SZ          10
#define MAX_DAYS_SALE       40
#define EEPROM_MAX_SIGS      8

#define USER_CHOICE_PRINT    (1<<0)

/* Available : 1K : 1024 */
struct ep_store_layout {
  uint8_t   item_mod_his_ptr;       /*             1 */
  item      item_mod_his[MOD_HIS_SZ]; /* 20*10=  200 */

  /* sale constants */
  uint16_t  sale_start;             /*             2 */
  uint16_t  sale_end;               /*             2 */
  uint16_t  vat[4];                 /*             8 */
  uint16_t  service_tax;            /*             2 */

  /* item constants
     max items = 64*8 = 512
   */
  uint16_t  item_last_modified;     /*             2 */
  uint16_t  item_count;             /*             2 */ /* */

  /* */
  uint16_t   passwd;                /*             2 */

  /* User choices */
  uint8_t   user_choice;            /*             1 */

  /* banners */
  uint8_t   shop_name [SHOP_NAME_SZ];  /*         16 */
  uint8_t   prn_header[HEADER_MAX_SZ]; /*         54 */
  uint8_t   prn_footer[FOOTER_MAX_SZ]; /*         24 */ /* */

  /* Store date and start id sale array */
  uint16_t  sale_start;
  uint16_t  sale_end;
  uint16_t  dmy[MAX_DAYS_SALE];        /*          2 */
  uint16_t  sales_idx[MAX_DAYS_SALE];  /*         96 */
  uint16_t  next_sale_idx;             /*          2 */
  uint16_t  num_sale_idx_free;         /*          2 */

  /* Store signature to depend on EEPROM_DATA */
  uint8_t   eeprom_idx;                   /*       1 */
  uint16_t  eeprom_sig[EEPROM_MAX_SIGS];  /*      16 */
};                                  /* Total  =  500 */

#define EEPROM_DATA               (*((struct ep_store_layout *)0))
#define EEPROM_STORE_READ         eepromReadBytes
#define EEPROM_STORE_WRITE_NoSig  eepromWriteBytes
#define EEPROM_STORE_WRITE(A, B, C) { 					\
    eepromWriteBytes(A, B, C);						\
    EEPROM_STORE_WRITE_Sig						\
  }
#define EEPROM_MAX_ADDR        (EEPROM_SIZE*EEPROM_HOWMANY)

#define EEPROM_STORE_WRITE_Sig {					\
    uint16_t ui1;	uint8_t ui2, eeprom_idx;			\
    EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.eeprom_idx), (uint8_t *)&eeprom_idx, sizeof(uint8_t)); \
    CRC16_Init();							\
    for (ui1=0; ui1<(uint16_t)&(EEPROM_DATA.eeprom_sig[0]); ui1++) {	\
      EEPROM_STORE_READ(ui1, &ui2, sizeof(uint8_t));			\
      CRC16_Update(ui2);						\
    }									\
    ui1 = CRC16_High; ui1 <<= 8; ui1 |= CRC16_Low;			\
    EEPROM_STORE_WRITE_NoSig((uint16_t)&(EEPROM_DATA.eeprom_sig[eeprom_idx]), (uint8_t *)&ui1, sizeof(uint16_t)); \
}

void ep_store_init(void);
void sale_index_it(sale_info *si, uint16_t ptr);

#endif
