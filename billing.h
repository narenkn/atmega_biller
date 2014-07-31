#ifndef BILLING_H
#define BILLING_H

#if 256 == BUFSS_SIZE
# define  SALE_INFO_ITEMS_NBITS    4
# define MAX_ITEMS_IN_BILL        16
#elif 512 == BUFSS_SIZE
# define  SALE_INFO_ITEMS_NBITS    5
# define MAX_ITEMS_IN_BILL        45
#endif

struct sale_item {
  uint16_t   ep_item_ptr;   /* pointer to item in EEPROM */
  uint16_t   quantity;      /* XXXX.YYY * 1000 */
  /* Option to hot override */
  uint16_t   cost;          /* XXXXX.YY * 100 */
  uint16_t   discount;      /* XXXXX.YY * 100 */
  uint8_t    unused:3;
  uint8_t    has_serv_tax:1;
  uint8_t    has_cess1:1;
  uint8_t    has_cess2:1;
  uint8_t    vat_sel:2;
  /* FIXME: Any change here should be reflected in menuBilling as well */
} __attribute__((packed)); /* 9 bytes */

/* */
#define  SALE_INFO_DELETED      0x8
#define  SALE_INFO_MODIFIED     0x4
struct sale_info {
  uint8_t   n_items:SALE_INFO_ITEMS_NBITS;
  uint8_t   property:(8-SALE_INFO_ITEMS_NBITS);

  uint16_t  date_yy:7;
  uint16_t  date_mm:4;
  uint16_t  date_dd:5;

  uint16_t  time_hh:5;
  uint16_t  time_mm:6;
  uint16_t  time_ss:5;
} __attribute__((packed));

struct sale {
  struct sale_info info;                 /*           5 */
  struct sale_item items[MAX_ITEMS_IN_BILL]; /*18*9=162       45*9 = 405     */
  uint32_t  taxable_total;               /*           4 */
  uint32_t  non_taxable_total;           /*           4 */
  uint32_t  cess1;                       /*           4 */
  uint32_t  cess2;                       /*           4 */
  uint32_t  discount;                    /*           4 */
  uint32_t  total;                       /*           4 */
  struct item      it[1];                /*          50 */
} __attribute__((packed));               /* Tot   = 241              484     */

/* constants */
#define SALE_SIZEOF       sizeof(struct sale)

#endif
