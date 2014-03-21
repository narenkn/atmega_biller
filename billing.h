#ifndef BILLING_H
#define BILLING_H

typedef struct {
  uint16_t   ep_ptr;   /* pointer to EEPROM */
  uint16_t   quantity; /* include 3 decimals */
  /* Option to hot override */
  uint16_t   cost;
  uint16_t   discount;
  uint8_t    unused1;
  uint8_t    unused:2;
  uint8_t    has_serv_tax:1;
  uint8_t    has_cess1:1;
  uint8_t    has_cess2:1;
  uint8_t    vat_sel:3;
} sale_item; /* 10 bytes */

/* 3 bytes */
#define  SALE_INFO_ITEMS_NBITS    5
#define  SALE_INFO_DELETED      0x8
#define  SALE_INFO_MODIFIED     0x4
typedef struct {
  uint8_t   n_items:SALE_INFO_ITEMS_NBITS;
  uint8_t   property:3;
  uint8_t   unused1;

  uint16_t  date_yy:7;
  uint16_t  date_mm:4;
  uint16_t  date_dd:5;

  uint16_t  time_hh:5;
  uint16_t  time_mm:6;
  uint16_t  time_ss:5;
} sale_info;  /* 6 bytes */
#define SALE_INFO_BYTE_NITEM_MASK   0xF0
#define SALE_INFO_BYTE_NITEM_SHIFT  4

typedef struct {
  sale_info info;                        /*           6 */
  sale_item items[MAX_ITEMS_IN_BILL];    /* 32*10 = 320 */
  uint32_t  taxable_total;
  uint32_t  non_taxable_total;
  uint32_t  cess1_total;
  uint32_t  cess2_total;
  uint32_t  final_discount;
  uint32_t  final_total;
} sale;                                  /* Tot   = 350 */

/* */
#define MAX_ITEMS_IN_BILL  (1<<SALE_INFO_ITEMS_NBITS)

/* constants */
#define SALE_INFO_SIZEOF  sizeof(sale_info)
#define SALE_ITEM_SIZEOF  sizeof(sale_item)

#endif
