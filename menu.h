#ifndef MENU_H
#define MENU_H

/* Documentation
   No. of users : EPS_MAX_USERS
   Super user   : 'admin'
 */


/* Type of Menu argument */
#define MENU_ITEM_NONE                 0
#define MENU_ITEM_ID                   1
#define MENU_ITEM_DATE                 2
#define MENU_ITEM_STR                  3
#define MENU_ITEM_MONTH                4
#define MENU_ITEM_FLOAT                5
#define MENU_ITEM_TIME                 6
#define MENU_ITEM_CONFIRM              7
#define MENU_ITEM_TYPE_MASK         0x1F
#define MENU_ITEM_OPTIONAL         (1<<7)
#define MENU_ITEM_DONTCARE_ON_PREV (1<<6)
#define MENU_ITEM_PASSWD           (1<<5)

/* Prompt for Menu argument */
#define MENU_PROMPT_LEN 5
#define MENU_PROMPTS \
  "None " /* 0 */    \
  "Id   " /* 1 */    \
  "Name " /* 2 */    \
  "OldPa" /* 3 */    \
  "Passw" /* 4 */    \
  "Month" /* 5 */    \
  "Time " /* 6 */    \
  "Date " /* 7 */    \
  "Decim" /* 8 */

#define MENU_PR_NONE   0
#define MENU_PR_ID     1
#define MENU_PR_NAME   2
#define MENU_PR_OLDPASS   3
#define MENU_PR_PASS   4
#define MENU_PR_MONTH  5
#define MENU_PR_TIME   6
#define MENU_PR_DATE   7
#define MENU_PR_FLOAT  8

typedef struct {
  union {
    struct {
      /* Total 24 bit max */
      uint8_t  i8;  /* msb */
      uint16_t i16;
    } integer;
    struct {
      uint8_t date;
      uint8_t month;
      uint8_t year;
    } date;
    struct {
      uint8_t min;
      uint8_t hour;
    } time;
  } value;
  uint8_t   valid;
} menu_arg_t;

#define MENU_NAMES_LEN 12

#define MENU_MSUPER    0xC0
#define MENU_MNORMAL   0x40
#define MENU_MRESET    0x80
#define MENU_MPRINT    0x01
#define MENU_MDELETE   0x02
#define MENU_MVALIDATE 0x03
#define MENU_MFOOTER   0x04
#define MENU_MDAYITEM  0x05
#define MENU_MDAYFULL  0x06
#define MENU_MDAYTAX   0x07
#define MENU_MMONITEM  0x08
#define MENU_MMONFULL  0x09
#define MENU_MMONTAX   0x0A
#define MENU_MALLITEM  0x0B
#define MENU_MALLFULL  0x0C
#define MENU_MALLTAX   0x0D
#define MENU_MODEMASK  0xE0

#define MENU_ITEMS \
  MENU_MODE(MENU_MNORMAL)  MENU_NAME("Billing     ") COL_JOIN MENU_FUNC(menu_Billing) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_NAME, MENU_ITEM_STR|MENU_ITEM_OPTIONAL|MENU_ITEM_DONTCARE_ON_PREV) ROW_JOIN \
  MENU_MODE(MENU_MNORMAL)  MENU_NAME("Show Bills  ") COL_JOIN MENU_FUNC(menu_ShowBill) COL_JOIN \
    ARG1(MENU_PR_DATE,  MENU_ITEM_DATE|MENU_ITEM_OPTIONAL) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_ID|MENU_ITEM_OPTIONAL) ROW_JOIN \
  MENU_MODE(MENU_MNORMAL|MENU_MPRINT)  MENU_NAME("Dup Bill    ") COL_JOIN MENU_FUNC(menu_ShowBill) COL_JOIN \
    ARG1(MENU_PR_DATE,  MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_ID) ROW_JOIN \
  MENU_MODE(MENU_MNORMAL|MENU_MDELETE)  MENU_NAME("Delete Bill ") COL_JOIN MENU_FUNC(menu_ShowBill) COL_JOIN \
    ARG1(MENU_PR_DATE,  MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_ID) ROW_JOIN \
  MENU_MODE(MENU_MSUPER) MENU_NAME("Add Item    ") COL_JOIN MENU_FUNC(menu_AddItem) COL_JOIN \
    ARG1(MENU_PR_NAME, MENU_ITEM_STR) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_MODE(MENU_MSUPER) MENU_NAME("Delete Item ") COL_JOIN MENU_FUNC(menu_DelItem) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_ID) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_MODE(MENU_MSUPER) MENU_NAME("Modify Item ") COL_JOIN MENU_FUNC(menu_ModItem) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_ID) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_MODE(MENU_MSUPER) MENU_NAME("SD Load Item") COL_JOIN MENU_FUNC(menu_SDLoadItem) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_ID) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_MODE(MENU_MSUPER) MENU_NAME("SD Save Item") COL_JOIN MENU_FUNC(menu_SDSaveItem) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_ID) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_MODE(MENU_MSUPER|MENU_MDAYITEM) MENU_NAME("Day ItemBill") COL_JOIN MENU_FUNC(menu_BillReports) COL_JOIN \
    ARG1(MENU_PR_DATE, MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_MODE(MENU_MSUPER|MENU_MDAYFULL) MENU_NAME("Day FullBill") COL_JOIN MENU_FUNC(menu_BillReports) COL_JOIN \
    ARG1(MENU_PR_DATE, MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  /* MENU_MODE(MENU_MSUPER|MENU_MDAY) MENU_NAME("DayBillPrned") COL_JOIN MENU_FUNC(menu_DayBillPrned) COL_JOIN */ \
  /*  ARG1(MENU_PR_DATE, MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN */ \
  /* MENU_MODE(MENU_MSUPER|MENU_MDAY) MENU_NAME("Day Dup Bill") COL_JOIN MENU_FUNC(menu_DayDupBill) COL_JOIN */ \
  /*  ARG1(MENU_PR_DATE, MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN */ \
  MENU_MODE(MENU_MSUPER|MENU_MDAYTAX|MENU_MMONTAX) MENU_NAME("Day Tax Repo") COL_JOIN MENU_FUNC(menu_BillReports) COL_JOIN \
    ARG1(MENU_PR_DATE, MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_MODE(MENU_MSUPER|MENU_MMONITEM) MENU_NAME("Mon ItemBill") COL_JOIN MENU_FUNC(menu_BillReports) COL_JOIN \
    ARG1(MENU_PR_MONTH , MENU_ITEM_MONTH) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_MODE(MENU_MSUPER|MENU_MMONFULL) MENU_NAME("Mon FullBill") COL_JOIN MENU_FUNC(menu_BillReports) COL_JOIN \
    ARG1(MENU_PR_MONTH, MENU_ITEM_MONTH) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  /* MENU_MODE(MENU_MSUPER|MENU_MMONTH) MENU_NAME("MonBillPrned") COL_JOIN MENU_FUNC(menu_MonBillPrned) COL_JOIN */ \
  /*  ARG1(MENU_PR_MONTH, MENU_ITEM_MONTH) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN */ \
  MENU_MODE(MENU_MSUPER|MENU_MMONTAX) MENU_NAME("Mon Tax Repo") COL_JOIN MENU_FUNC(menu_BillReports) COL_JOIN \
    ARG1(MENU_PR_MONTH, MENU_ITEM_MONTH) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_MODE(MENU_MSUPER|MENU_MALLTAX) MENU_NAME("All ItemBill") COL_JOIN MENU_FUNC(menu_BillReports) COL_JOIN \
    ARG1(MENU_PR_DATE, MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_DATE, MENU_ITEM_DATE) ROW_JOIN \
  MENU_MODE(MENU_MSUPER|MENU_MALLFULL) MENU_NAME("All FullBill") COL_JOIN MENU_FUNC(menu_BillReports) COL_JOIN \
    ARG1(MENU_PR_DATE, MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_DATE, MENU_ITEM_DATE) ROW_JOIN \
  /* MENU_MODE(MENU_MSUPER) MENU_NAME("All Tax Repo") COL_JOIN MENU_FUNC(menu_TaxReport) COL_JOIN */ \
  /*  ARG1(MENU_PR_DATE, MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_DATE, MENU_ITEM_DATE) ROW_JOIN */ \
  MENU_MODE(MENU_MSUPER) MENU_NAME("Modify VAT  ") COL_JOIN MENU_FUNC(menu_ModVat) COL_JOIN \
    ARG1(MENU_PR_FLOAT, MENU_ITEM_FLOAT) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_MODE(MENU_MSUPER) MENU_NAME("Set Header  ") COL_JOIN MENU_FUNC(menu_Header) COL_JOIN \
    ARG1(MENU_PR_NAME, MENU_ITEM_STR) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_MODE(MENU_MSUPER|MENU_MFOOTER) MENU_NAME("Set Footer  ") COL_JOIN MENU_FUNC(menu_Header) COL_JOIN \
    ARG1(MENU_PR_NAME, MENU_ITEM_STR) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_MODE(MENU_MSUPER) MENU_NAME("Del All Bill") COL_JOIN MENU_FUNC(menu_DelAllBill) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_MODE(MENU_MSUPER|MENU_MNORMAL) MENU_NAME("Change Passw") COL_JOIN MENU_FUNC(menu_SetPasswd) COL_JOIN \
    ARG1(MENU_PR_OLDPASS, MENU_ITEM_STR|MENU_ITEM_PASSWD) COL_JOIN ARG2(MENU_PR_PASS, MENU_ITEM_STR|MENU_ITEM_PASSWD) ROW_JOIN \
  MENU_MODE(MENU_MSUPER) MENU_NAME("Set Serv Tax") COL_JOIN MENU_FUNC(menu_SetServTax) COL_JOIN \
    ARG1(MENU_PR_FLOAT, MENU_ITEM_FLOAT) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_MODE(MENU_MSUPER) MENU_NAME("Set DateTime") COL_JOIN MENU_FUNC(menu_SetDateTime) COL_JOIN \
    ARG1(MENU_PR_DATE, MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_TIME, MENU_ITEM_TIME) ROW_JOIN \
  MENU_MODE(MENU_MSUPER)  MENU_NAME("Chg Usr/Pass") COL_JOIN MENU_FUNC(menu_SetUserPasswd) COL_JOIN \
    ARG1(MENU_PR_NAME,  MENU_ITEM_STR) COL_JOIN ARG2(MENU_PR_PASS,  MENU_ITEM_STR|MENU_ITEM_PASSWD) ROW_JOIN \
  MENU_MODE(MENU_MSUPER|MENU_MNORMAL)  MENU_NAME("User  Logout") COL_JOIN MENU_FUNC(menu_UserLogout) COL_JOIN \
    ARG1(MENU_PR_ID,  MENU_ITEM_CONFIRM) COL_JOIN ARG2(MENU_PR_ID,  MENU_ITEM_NONE) ROW_JOIN \
  MENU_MODE(MENU_MRESET)  MENU_NAME("User Login  ") COL_JOIN MENU_FUNC(menu_UserLogin) COL_JOIN \
    ARG1(MENU_PR_NAME,  MENU_ITEM_STR) COL_JOIN ARG2(MENU_PR_PASS,  MENU_ITEM_STR|MENU_ITEM_PASSWD) ROW_JOIN \
  MENU_MODE(MENU_MRESET) MENU_NAME("Run Diagnost") COL_JOIN MENU_FUNC(menu_RunDiag) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE)

/* Let's check if this buf size if good enough */
#define  BUFSS_SIZE   256

extern uint8_t menu_error;
extern uint8_t bufSS[BUFSS_SIZE];

/* Helper routines */
void menu_getopt(uint8_t *prompt, menu_arg_t *arg, uint8_t opt);
uint8_t menu_getchoice(uint8_t *quest, uint8_t *opt_arr, uint8_t max_idx);
//void menu_Header(uint8_t mode);
//void menu_sale_add(uint8_t *sale);
//void menu_sale_delete_month(uint8_t del_month);
//uint16_t menu_sale_find(uint8_t *dmy, uint16_t id);
//void menu_sale_free_old_sector(void);

/* User routines*/
void menu_SetPasswd(uint8_t mode);
void menu_SetUserPasswd(uint8_t mode);
void menu_UserLogout(uint8_t mode);
void menu_UserLogin(uint8_t mode);

/* Item routines */
void menu_AddItem(uint8_t mode);
void menu_DelItem(uint8_t mode);
void menu_ModItem(uint8_t mode);

/* billing routines */
void menu_Billing(uint8_t mode);
void menu_ShowBill(uint8_t mode);

/* User option routines */
//void menu_PrnHeader(void);
//void menu_PrnFooter(void);
//void menu_ModVat(uint8_t mode);
//void menu_SetServTax(uint8_t mode);
//void menu_SetDateTime(uint8_t mode);

/* Report routines */
//void menu_PrnFullBill(billing *bp);
//void menu_BillReports(uint8_t mode);
//void menu_PrnItemBill(billing *bp);
//void menu_PrnItemBillFooter(void);
//void menu_PrnAllBill(billing *bp);
//void menu_PrnTaxReport(billing *bp);
//void menu_PrnTaxReportFooter(billing *bp);
//void menu_DelAllBill(uint8_t mode);

/* Other routines */
//void menu_RunDiag(uint8_t mode);
//void menu_main(void);

#define menu_item_find(id) ((id<ITEM_MAX) ? (ITEM_SIZEOF*id): EEPROM_ADDR_INVALID)

#endif
