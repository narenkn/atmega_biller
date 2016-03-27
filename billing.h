#ifndef BILLING_H
#define BILLING_H

#if BUFSS_SIZE < 512
# define MAX_ITEMS_IN_BILL        16
#elif 512 == BUFSS_SIZE
# define MAX_ITEMS_IN_BILL        45
#endif

struct sale_item {
  uint16_t   ep_item_ptr;   /* pointer to item in EEPROM */
  uint16_t   quantity;      /* XXXX.YYY * 1000 */
  /* Option to hot override */
  uint32_t   cost;          /* XXXXX.YY * 100 */
  uint32_t   discount;      /* XXXXX.YY * 100 */
  uint8_t    unused:2;
  uint8_t    has_common_discount:1;
  uint8_t    has_vat:1;
  uint8_t    has_serv_tax:1;
  uint8_t    vat_sel:3;
  /* FIXME: Any change here should be reflected in menuBilling as well */
} __attribute__((packed)); /* 13 bytes */

/* */
#define  SALE_INFO_DELETED      0x8
#define  SALE_INFO_MODIFIED     0x4
#define  SALE_INFO_INVALID      0x1
struct sale_info {
  uint8_t   n_items;
  uint8_t   property;
#if MENU_USER_ENABLE
  uint8_t   user[EPS_MAX_UNAME];
#endif

  uint16_t  bill_id;

  uint16_t  date_yy:7;
  uint16_t  date_mm:4;
  uint16_t  date_dd:5;

  uint16_t  time_hh:5;
  uint16_t  time_mm:6;
  uint16_t  time_ss:5;
} __attribute__((packed));

struct sale {
  uint16_t  crc_invert;                  /*           2 */
  uint16_t  crc;                         /*           2 */
  struct sale_info info;                 /*        16/8 */
  struct sale_item items[MAX_ITEMS_IN_BILL];/*16*13=208       45*14 = 630 */
  uint32_t  t_stax;                      /*           4 */
  uint32_t  t_discount;                  /*           4 */
  uint32_t  t_vat;                       /*           4 */
  uint32_t  total;                       /*           4 */
  struct item      it[1];                /*           0 (not stored) */
} __attribute__((packed));               /* Tot=244/236               662  */

/* constants */
#define SALE_SIZEOF       sizeof(struct sale)
#define SIZEOF_1BILL      (SALE_SIZEOF-ITEM_SIZEOF+(ITEM_SIZEOF*MAX_ITEMS_IN_BILL))
#define SALE_DATA_EXP_ITEMS_SIZEOF  (sizeof(struct sale)-ITEM_SIZEOF)

void billingInit(void);

/* Manage items in EEPROM */
#define itemAddr(id) (((uint16_t)ITEM_SIZEOF>>2)*(id-1))
#define itemId(addr) ({					\
      uint16_t _ret, _ret1=addr;			\
      for (_ret=1; _ret1 > ((ITEM_MAX*ITEM_SIZEOF)>>2);	\
	   _ret++, _ret1-=ITEM_SIZEOF);			\
      _ret;						\
    })
#define ITEM_MAX_ADDR ((ITEM_MAX*(ITEM_SIZEOF+4))>>2)
#define ITEM_IDX_START ((ITEM_SIZEOF>>2)*(ITEM_MAX))
#define itemIdxAddr(id) (ITEM_IDX_START+id-1)

/* Biggest device that can be connected to Board/rev0 is 4*24C512
   which is 4*64KBytes = 128KBytes
   * item's size is 56 bytes. Max items 1024. So need 56Kbytes.
   * We still have 3 more EEPROM's of ~3*64KB = 192KBytes + 8KBytes
   * Even if one sale is of size 256 bytes, we can store 200*1024/248
                                                          = 825 bills

   If address stored in 16-bits
   4*64KB == 256KBytes == 2^^18, so SALE should be 4 byte aligned.
 */
#define SALE_DATA_SIZEOF_NORM    (SALE_DATA_EXP_ITEMS_SIZEOF>>2)
#define EEPROM_SALE_START_ADDR   ITEM_MAX_ADDR
#define EEPROM_BILL_ADDR(N)      (EEPROM_SALE_START_ADDR+	\
				  (SALE_DATA_SIZEOF_NORM*N))
#define EEPROM_SALE_END_APPROX_ADDR		\
  (EEPROM_MAX_ADDRESS-SALE_DATA_SIZEOF_NORM+1)
#define EEPROM_SALE_MAX_BILLS						\
  ((uint16_t)( ((EEPROM_SALE_END_APPROX_ADDR-EEPROM_SALE_START_ADDR)	\
		/SALE_DATA_SIZEOF_NORM)) + 1 )
#define EEPROM_SALE_END_ADDR /* could be last-avail-addr+1 */		\
  (EEPROM_SALE_START_ADDR+(SALE_DATA_SIZEOF_NORM*(EEPROM_SALE_MAX_BILLS-1)))
#define EEPROM_PREV_SALE_RECORD(curAddr)			\
  ( ((curAddr-SALE_DATA_SIZEOF_NORM)<EEPROM_SALE_START_ADDR) ?	\
    EEPROM_SALE_END_ADDR : (curAddr-SALE_DATA_SIZEOF_NORM) )
#define EEPROM_NEXT_SALE_RECORD(curAddr)			\
  ( ((curAddr+SALE_DATA_SIZEOF_NORM)<=EEPROM_SALE_END_ADDR) ?	\
    (curAddr+SALE_DATA_SIZEOF_NORM) : EEPROM_SALE_START_ADDR )

#endif
