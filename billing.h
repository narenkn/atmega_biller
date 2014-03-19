#ifndef BILLING_H
#define BILLING_H

/* Balanced to 20 bytes */
#define ITEM_NAME_BYTEL        15
typedef struct {
  uint16_t  unused:2;
  uint16_t  cost:13;
  uint16_t  id:9;
  uint16_t  has_serv_tax:1;
  uint16_t  vat_sel:2;
  uint16_t  discount:13;
  uint8_t   name[ITEM_NAME_BYTEL];
} item;
#define  ITEM_BYTE_VALID_MASK   (1<<6)
#define  ITEM_BYTE_DELETE_MASK  (1<<5)
#define  ITEM_BYTE_ID_H_MASK    (1<<7)
#define  ITEM_BYTE_ID_H_ARRANGE_SHIFT_NUM 1
#define  ITEM_BYTE_ID_BYTE_OFFSET         2

/* 2 bytes */
typedef struct {
  uint16_t   item_id:9;
  uint16_t   unused:3;
  uint16_t   num_sale:4;
} sale_item;

/* 3 bytes */
#define  SALE_INFO_ITEMS_NBITS     4
#define  SALE_INFO_DELETED      0x80
#define  SALE_INFO_MODIFIED     0x40
typedef struct _sale_info {
  uint16_t  n_items:SALE_INFO_ITEMS_NBITS;

  uint16_t  time_hh:5;
  uint16_t  time_mm:6;

  uint16_t  date_dd:5;
  uint16_t  date_mm:4;
  uint8_t   property;
} sale_info;
//#define SALE_INFO_P             ((struct sale_info *)0)
#define SALE_INFO               (*((sale_info *)0))
//#define SALE_INFO                   ((__code sale_info *)0x10)
#define SALE_INFO_BYTE_NITEM_MASK   0xF0
#define SALE_INFO_BYTE_NITEM_SHIFT  4

typedef struct {
  sale_info info;                        /*         4 */
  sale_item items[MAX_ITEMS_IN_BILL]     /* 2*16 = 32 */
} sale;                                  /* Tot  = 36 */

typedef struct {
  uint8_t   vat_sel:2;
  uint8_t   has_serv_tax:1;
  uint8_t   unused:5;
} bill_info;

/* */
#define MAX_ITEMS_IN_BILL  (1<<SALE_INFO_ITEMS_NBITS)
#define BILLING_PRINT      (1<<0)
typedef struct {
  sale_info info;                        /*         4 */
  sale_item items[MAX_ITEMS_IN_BILL];    /* 2*16 = 32 */
  bill_info bi[MAX_ITEMS_IN_BILL];       /* 1*16 = 16 */
  item      temp;                        /*        19 */
  uint16_t  ui1, ui3, ui5, ui7;          /* 2*4  =  8 */
  uint8_t   ui2, ui4, ui6, flags;        /*         3 */
  uint16_t  ui9, ui11, ui13, ui15;       /* 2*4  =  8 */
} billing;

/* constants */
#define ITEM_SIZEOF       sizeof(item)
#define SALE_INFO_SIZEOF  sizeof(sale_info)
#define SALE_SIZEOF       sizeof(sale_item)

#endif
