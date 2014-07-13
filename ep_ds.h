#ifdef EP_DS_H
#define EP_DS_H

#define       ITEM_NAME_UNI_BYTEL    24
#define           ITEM_NAME_BYTEL    12
#define      ITEM_PROD_CODE_BYTEL    20

typedef struct {
    uint16_t                   discout;
    uint16_t                      cost;
    uint16_t                        id;
     uint8_t              has_serv_tax:1;
     uint8_t                 prod_code[ITEM_PROD_CODE_BYTEL];
     uint8_t              name_unicode[ITEM_NAME_UNI_BYTEL];
     uint8_t                      name[ITEM_NAME_BYTEL];
     uint8_t                   vat_sel:3;
     uint8_t                 has_cess2:1;
     uint8_t           has_weighing_mc:1;
     uint8_t           name_in_unicode:1;
     uint8_t                 has_cess1:1;
     uint8_t            is_biller_item:1;
     uint8_t       has_common_discount:1;
} __attribute__((packed))       item;


#endif
#ifdef EP_DS_H
#define EP_DS_H

#define             EPS_MAX_UNAME     8
#define         HEADER_MAX_SZ_MAX    54
#define         FOOTER_MAX_SZ_MAX    24
#define          SHOP_NAME_SZ_MAX    16
#define        EPS_MAX_VAT_CHOICE     8
#define             EPS_MAX_USERS    15

typedef struct {
    uint16_t               service_tax;
    uint16_t            prn_header_len;
    uint16_t                     cess2;
    uint16_t                     cess1;
    uint16_t                 round_off;
    uint16_t                   passwds[EPS_MAX_UNAME+1];
    uint16_t                  property;
    uint16_t            prn_footer_len;
    uint16_t                       vat[EPS_MAX_VAT_CHOICE];
    uint16_t             shop_name_len;
     uint8_t         denomination_text[20];
     uint8_t                prn_footer[FOOTER_MAX_SZ_MAX];
     uint8_t                     users[EPS_MAX_USERS][EPS_MAX_UNAME];
     uint8_t                prn_header[HEADER_MAX_SZ_MAX];
     uint8_t                 shop_name[SHOP_NAME_SZ_MAX];
} __attribute__((packed)) ep_store_layout;


#endif
