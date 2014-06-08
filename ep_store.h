#ifndef EP_STORE_H
#define EP_STORE_H

#define HEADER_MAX_SZ_MAX      54
#define SHOP_NAME_SZ_MAX       16
#define FOOTER_MAX_SZ_MAX      24

#define EPS_MAX_VAT_CHOICE     8
#define EPS_MAX_USERS          15
#define EPS_MAX_UNAME          8

/* Available min : 1K : 1024 */
struct ep_store_layout {
  /* sale constants */
  uint16_t  vat[EPS_MAX_VAT_CHOICE];   /*8 * 2 =  16 */ /* in % * 100 */
  uint16_t  service_tax;               /*          2 */ /* in % * 100 */
  uint16_t  cess1;                     /*          2 */ /* in % * 100 */
  uint16_t  cess2;                     /*          2 */ /* in % * 100 */

  /* Sale options */
  uint8_t   denomination_text[20];     /*         20 */ /* 42 */
  uint16_t  round_off;                 /*          2 */
  /* bits [15:4] : Unused
     bits [3:0] : Printing
       value 0b0000 : don't ask, 0b00xx : copies without confirmation,
       0b01xx : confirm before print xx copies
  */
  uint16_t   property;                  /*         2 */

  /* User options
     User 0 : is 'admin' + 15 usernames
     16 passwords
   */
  uint8_t  users[EPS_MAX_USERS][EPS_MAX_UNAME];/*120 */
  uint16_t passwds[EPS_MAX_USERS+1];    /*        32 */ /* 182 */

  /* banners */
  uint16_t  shop_name_len;             /*          2 */
  uint16_t  prn_header_len;            /*          2 */
  uint16_t  prn_footer_len;            /*          2 */
  uint8_t   shop_name [SHOP_NAME_SZ_MAX];  /*         16 */ /* 204 */
  uint8_t   prn_header[HEADER_MAX_SZ_MAX]; /*         54 */
  uint8_t   prn_footer[FOOTER_MAX_SZ_MAX]; /*         24 */
} __attribute__((packed));              /* Total  =  282 */

/* Balanced to 64 bytes */
#define ITEM_NAME_BYTEL        12
#define ITEM_NAME_UNI_BYTEL    24
#define ITEM_PROD_CODE_BYTEL   20
typedef struct {
  /* Non-zero id means a valid item */
  uint16_t  id;                              /*  2 */
  uint8_t   name[ITEM_NAME_BYTEL];           /* 12 */
  uint8_t   prod_code[ITEM_PROD_CODE_BYTEL]; /* 20 */
  uint16_t  cost;                            /*  2 */ /* XXXXX.YY * 100 */
  uint16_t  discount;                        /*  2 */ /* XXXXX.YY * 100 */
  uint8_t   name_unicode[ITEM_NAME_UNI_BYTEL];/*24 */
  uint8_t   has_serv_tax:1;                  /*  1 */
  uint8_t   has_common_discount:1;
  uint8_t   has_cess1:1;
  uint8_t   has_cess2:1;
  uint8_t   has_weighing_mc:1;
  uint8_t   vat_sel:3;
  uint8_t   name_in_unicode:1;               /*  1 */
  uint8_t   is_biller_item:1;
  uint8_t   unused:6;
} __attribute__((packed)) item; /* Total     :  64 */
#define ITEM_SIZEOF sizeof(item)

/* EEPROM constants */
#define EEPROM_DEVICE_SIZE      ((uint32_t)(1<<10<<6))
#define EEPROM_NUM_DEVICES      2
/* 0:8-bits; 1:16-bits; 2:32-bits */
#define EEPROM_ADDR_LSB_IGNORE  1

/* */
#define EEPROM_ADDR_INVALID     0xFFFF

/* Current Properties */
uint8_t user_id;
uint8_t user_logged_in;

/* Access routines */
void ep_store_init(void);

#endif
