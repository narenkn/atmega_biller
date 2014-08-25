#ifndef BILLING_H
#define BILLING_H

#if 256 == BUFSS_SIZE
# define MAX_ITEMS_IN_BILL         8
#elif 512 == BUFSS_SIZE
# define MAX_ITEMS_IN_BILL        45
#endif

struct sale_item {
  uint16_t   ep_item_ptr;   /* pointer to item in EEPROM */
  uint16_t   quantity;      /* XXXX.YYY * 1000 */
  /* Option to hot override */
  uint32_t   cost;          /* XXXXX.YY * 100 */
  uint32_t   discount;      /* XXXXX.YY * 100 */
  uint8_t    unused_0;
  uint8_t    unused:2;
  uint8_t    has_common_discount:1;
  uint8_t    has_vat:1;
  uint8_t    has_serv_tax:1;
  uint8_t    vat_sel:3;
  /* FIXME: Any change here should be reflected in menuBilling as well */
} __attribute__((packed)); /* 14 bytes */

/* */
#define  SALE_INFO_DELETED      0x8
#define  SALE_INFO_MODIFIED     0x4
#define  SALE_INFO_INVALID      0x1
struct sale_info {
  uint8_t   n_items;
  uint8_t   prop;
  uint8_t   user[EPS_MAX_UNAME];

  uint16_t  date_yy:7;
  uint16_t  date_mm:4;
  uint16_t  date_dd:5;

  uint16_t  time_hh:5;
  uint16_t  time_mm:6;
  uint16_t  time_ss:5;
} __attribute__((packed));

struct sale {
  struct sale_info info;                 /*           6 */
  struct sale_item items[MAX_ITEMS_IN_BILL]; /*8*14=112       45*9 = 405     */
  uint32_t  t_stax;                      /*           4 */
  uint32_t  t_discount;                  /*           4 */
  uint32_t  t_vat;                       /*           4 */
  uint32_t  total;                       /*           4 */
  struct item      it[1];                /*          56 */
} __attribute__((packed));               /* Tot   = 190              484     */

/* constants */
#define SALE_SIZEOF       sizeof(struct sale)

#endif
