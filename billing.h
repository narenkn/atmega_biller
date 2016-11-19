#ifndef BILLING_H
#define BILLING_H

struct sale_item {
  uint32_t   quantity;      /* XXXX.YYY * 1000 could be in grams */
  /* Option to hot override */
  uint32_t   cost;          /* XXXXX.YY * 100 */
  uint32_t   discount;      /* XXXXX.YY * 100 */
  uint16_t   ep_item_ptr;   /* pointer to item in EEPROM */
  uint8_t    unused0:1;
  uint8_t    has_common_discount:1;
  uint8_t    has_vat:1;
  uint8_t    has_tax1:1;
  uint8_t    has_tax2:1;
  uint8_t    has_tax3:1;
  uint8_t    is_reverse_tax:1;
  uint8_t    has_weighing_mc:1;
  /* FIXME: Any change here should be reflected in menuBilling as well */
} __attribute__((packed)); /* 15 bytes */

/* */
struct sale_info {
  uint8_t   n_items;
  uint8_t   user[EPS_MAX_UNAME];
  uint16_t  id;

  uint16_t  is_deleted:1;
  uint16_t  dup_bill_issued:1;
  uint16_t  is_void:1;
  uint16_t  date_yy:12;
  uint16_t  date_mm:4;
  uint16_t  date_dd:5;

  uint16_t  time_hh:5;
  uint16_t  time_mm:6;
  uint16_t  time_ss:5;
} __attribute__((packed));

#define MAX_SIZEOF_1BILL         1024
#define MAX_ITEMS_IN_BILL        ((MAX_SIZEOF_1BILL-4-17-32)/sizeof(struct sale_item))
struct sale {
  uint16_t  crc_invert;                  /*             2 */
  uint16_t  crc;                         /*             2 */
  struct sale_info info;                 /*            16 */
  uint8_t   tableNo;                     /*             1 */
  /* how may bills inside 1024 bytes ?
     1024 - 32 - 12 = 980
     980 / 13 = 75 */
  struct sale_item items[MAX_ITEMS_IN_BILL];/* 69*15=1104 */
  uint32_t  t_tax1;                      /*             4 */
  uint32_t  t_tax2;                      /*             4 */
  uint32_t  t_tax3;                      /*             4 */
  uint32_t  t_vat;                       /*             4 */
  uint32_t  t_discount;                  /*             4 */
  uint32_t  total;                       /*             4 */
  uint32_t  t_cash_pay;                  /*             4 */
  uint32_t  t_other_pay;                 /*             4 */
  struct item      it[1];                /*             0 (not stored) */
} __attribute__((packed));               /* Tot=     1020 */

/* constants */
#define SIZEOF_SALE       sizeof(struct sale) /* 1020+52=1072 */
#define SIZEOF_SALE_EXCEP_ITEMS  (SIZEOF_SALE-ITEM_SIZEOF) /* 1020 */

void billingInit(void);

/* Manage items in EEPROM */
#define itemAddr(id) (((uint16_t)ITEM_SIZEOF>>2)*(id-1))
#define itemId(addr) ({					\
      uint16_t _ret, _ret1=addr;			\
      for (_ret=1; _ret1 >= (ITEM_SIZEOF>>2);		\
	   _ret++, _ret1-=(ITEM_SIZEOF>>2));		\
      (addr < ITEM_MAX_ADDR) ? _ret : ITEM_MAX;		\
    })
#define itemNxtAddr(addr) \
  (((addr+(ITEM_SIZEOF>>2))<ITEM_MAX_ADDR) ? (addr+(ITEM_SIZEOF>>2)) : 0)
#define itemNxtId(id) \
  ((id<ITEM_MAX) ? id+1 : 1)
#define ITEM_MAX_ADDR (ITEM_MAX*(ITEM_SIZEOF>>2))

#if ! NVFLASH_EN

/* Biggest device that can be connected to Board/rev0 is 4*24C512
   which is 4*64KBytes = 128KBytes
   * item's size is 56 bytes. Max items 1024. So need 56Kbytes.
   * We still have 3 more EEPROM's of ~3*64KB = 192KBytes + 8KBytes
   * Even if one sale is of size 256 bytes, we can store 200*1024/248
                                                          = 825 bills

   If address stored in 16-bits
   4*64KB == 256KBytes == 2^^18, so SALE should be 4 byte aligned.
 */
#define SALE_DATA_SIZEOF_NORM    (SIZEOF_SALE_EXCEP_ITEMS>>2)
#define EEPROM_SALE_START_ADDR   ITEM_MAX_ADDR
#define EEPROM_BILL_ADDR(N)      (EEPROM_SALE_START_ADDR+	\
				  (SALE_DATA_SIZEOF_NORM*N))
#define EEPROM_SALE_END_APPROX_ADDR		\
  ((uint16_t)(EEPROM_MAX_ADDRESS-SALE_DATA_SIZEOF_NORM+1))
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
#define EEPROM_BILL_HAS_ITEMS    0

#else /* #if ! NVFLASH_EN */

/* NV Flash : Cypress 64Mbits/8M bytes
   Max devices : 4
   Total addr space is 32Mbytes, need 25[24:0] addr bits. But we're using
     16-bit pointers. So, each block needs to be of size (25-16)=9 bits
     wide (atleast).
*/

#define NVF_MIN_BLOCK_SZ         (1<<9)
#define SALE_DATA_SIZEOF_NORM    ((SIZEOF_SALE_EXCEP_ITEMS/NVF_MIN_BLOCK_SZ)+((SIZEOF_SALE_EXCEP_ITEMS&(NVF_MIN_BLOCK_SZ-1)) ? 1 : 0)) /* how many NVF_MIN_BLOCK_SZ? */
#define EEPROM_SALE_START_ADDR   0
#define EEPROM_BILL_ADDR(N)      (N<<(SALE_DATA_SIZEOF_NORM-1))
#define EEPROM_MAX_ADDRESS     ((uint16_t)((0x3FFF<<EEPROM_MAX_DEVICES_LOGN2)|0xF)&0xFFFF)

#define EEPROM_SALE_MAX_BILLS						\
  ((uint16_t)((EEPROM_MAX_ADDRESS+1-EEPROM_SALE_START_ADDR)/SALE_DATA_SIZEOF_NORM))
#define EEPROM_SALE_END_ADDR /* could be last-avail-addr+1 */		\
  (EEPROM_SALE_START_ADDR+(SALE_DATA_SIZEOF_NORM*(EEPROM_SALE_MAX_BILLS-1)))

#define EEPROM_PREV_SALE_RECORD(curAddr)			\
  ( (curAddr==EEPROM_SALE_START_ADDR) ?				\
    EEPROM_SALE_END_ADDR : (curAddr-SALE_DATA_SIZEOF_NORM) )
#define EEPROM_NEXT_SALE_RECORD(curAddr)			\
  ( ((curAddr+SALE_DATA_SIZEOF_NORM)<=EEPROM_SALE_END_ADDR) ?	\
    (curAddr+SALE_DATA_SIZEOF_NORM) : EEPROM_SALE_START_ADDR )

#define EEPROM_BILL_HAS_ITEMS    0

#endif

#endif
