#ifndef EP_STORE_H
#define EP_STORE_H

#define HEADER_MAX_SZ       54
#define SHOP_NAME_SZ        16
#define FOOTER_MAX_SZ       24

#define USER_CHOICE_PRINT    (1<<0)
#define EPS_MAX_VAT_CHOICE   8
#define EPS_MAX_USERS        8
#define EPS_MAX_UNAME        8
#define ITEM_MAX             256

/* Available : 1K : 1024 */
struct ep_store_layout {
  /* sale constants */
  uint16_t  vat[EPS_MAX_VAT_CHOICE];/*   8 * 2 =  16 */
  uint16_t  service_tax;            /*             2 */
  uint16_t  cess;                   /*             2 */

  /* Sale options */
  uint8_t   denomination_text[6];   /*             6 */ /* 26 */

  /* User options */
  uint8_t   users[EPS_MAX_USERS][EPS_MAX_UNAME];/*64 */
  uint16_t  passwds[EPS_MAX_USERS]; /*            16 */
  uint16_t  round_off;              /*             2 */ /* 108 */

  /* banners */
  uint8_t   shop_name [SHOP_NAME_SZ];  /*         16 */
  uint8_t   prn_header[HEADER_MAX_SZ]; /*         54 */
  uint8_t   prn_footer[FOOTER_MAX_SZ]; /*         24 */
};                                  /* Total  =  202 */

/* Balanced to 64 bytes */
#define ITEM_NAME_BYTEL        18
#define ITEM_NAME_UNI_BYTEL    38
typedef struct {
  uint16_t  has_serv_tax:1;                  /*  2 */
  uint16_t  has_cess:1;
  uint16_t  vat_sel:3;
  uint16_t  valid:1;
  uint16_t  unused:10;
  uint16_t  cost;                            /*  2 */
  uint16_t  discount;                        /*  2 */
  uint16_t  id;                              /*  2 */
  uint8_t   name[ITEM_NAME_BYTEL];           /* 18 */
  uint8_t   name_uni[ITEM_NAME_UNI_BYTEL];   /* 38 */
} item;
#define ITEM_SIZEOF sizeof(item)

void ep_store_init(void);

#endif
