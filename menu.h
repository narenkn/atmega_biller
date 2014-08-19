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
#define MENU_PROMPT_LEN 8
#define MENU_PROMPTS \
  "None    " /* 0 */    \
  "Id      " /* 1 */    \
  "Name    " /* 2 */    \
  "OldPaswd" /* 3 */    \
  "Password" /* 4 */    \
  "Month   " /* 5 */    \
  "Time    " /* 6 */    \
  "Date    " /* 7 */    \
  "Decimal " /* 8 */

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
      uint8_t day;
      uint8_t month;
      uint8_t year;
    } date;
    struct {
      uint8_t min;
      uint8_t hour;
    } time;
    uint8_t *sptr;
  } value;
  uint8_t   valid;
} menu_arg_t;

#define MENU_NAMES_LEN 12

/* One super-user and other billing users */
#define MENU_MSUPER    0x20
#define MENU_MNORMAL   0x40  /* User functions */
#define MENU_MRESET    0x80  /* Right after reset */
#define MENU_MODITEM   0x01
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

/* Implementation of Hierarchical menu : Every menu item could be associated
   with a main menu */
#define MENU_HIER_BILLING    1
#define MENU_HIER_SETTINGS   2
#define MENU_HIER_HISTORY    3
#define MENU_HIER_SD         4
#define MENU_HIER_DIAGNOSTIC 5
#define MENU_HIER_MAX        6
#define MENU_HIER_NAME_SIZE  7
#define MENU_HIER_NAMES      "Billing" \
                             "Setting" \
                             "History" \
                             "SD Card" \
                             "Diagnos"

#define MENU_VAR_TYPE_NONE   0
#define MENU_VAR_TYPE_U8     1
#define MENU_VAR_TYPE_U16    2
#define MENU_VAR_TYPE_U32    3
#define MENU_VAR_TYPE_FLOAT  4
#define MENU_VAR_TYPE_STRING 5
#define MENU_VAR_TYPE_CHOICE 6

struct menu_vars {
uint8_t type;
uint8_t size;
uint8_t size2;
uint8_t name[9];
uint16_t ep_ptr;
};

#define MENU_ITEMS \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER|MENU_MNORMAL)  MENU_NAME("Billing     ") COL_JOIN MENU_FUNC(menuBilling) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_NAME, MENU_ITEM_STR|MENU_ITEM_OPTIONAL|MENU_ITEM_DONTCARE_ON_PREV) ROW_JOIN \
  MENU_HIER(MENU_HIER_HISTORY) MENU_MODE(MENU_MSUPER|MENU_MNORMAL)  MENU_NAME("Show Bills  ") COL_JOIN MENU_FUNC(menuShowBill) COL_JOIN \
    ARG1(MENU_PR_DATE,  MENU_ITEM_DATE|MENU_ITEM_OPTIONAL) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_ID|MENU_ITEM_OPTIONAL) ROW_JOIN \
  MENU_HIER(MENU_HIER_HISTORY) MENU_MODE(MENU_MSUPER|MENU_MNORMAL|MENU_MPRINT)  MENU_NAME("Dup Bill    ") COL_JOIN MENU_FUNC(menuShowBill) COL_JOIN \
    ARG1(MENU_PR_DATE,  MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_ID) ROW_JOIN \
  MENU_HIER(MENU_HIER_HISTORY) MENU_MODE(MENU_MSUPER|MENU_MDELETE)  MENU_NAME("Delete Bill ") COL_JOIN MENU_FUNC(menuShowBill) COL_JOIN \
    ARG1(MENU_PR_DATE,  MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_ID) ROW_JOIN \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER) MENU_NAME("Add Item    ") COL_JOIN MENU_FUNC(menuAddItem) COL_JOIN \
    ARG1(MENU_PR_NAME, MENU_ITEM_STR) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER) MENU_NAME("Delete Item ") COL_JOIN MENU_FUNC(menuDelItem) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_ID) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER|MENU_MODITEM) MENU_NAME("Modify Item ") COL_JOIN MENU_FUNC(menuAddItem) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_FLOAT) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_SETTINGS) MENU_MODE(MENU_MSUPER) MENU_NAME("SD Load Item") COL_JOIN MENU_FUNC(menuSDLoadItem) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_ID) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_SETTINGS) MENU_MODE(MENU_MSUPER) MENU_NAME("SD Save Item") COL_JOIN MENU_FUNC(menuSDSaveItem) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_ID) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_HISTORY) MENU_MODE(MENU_MSUPER|MENU_MDAYITEM) MENU_NAME("Day ItemBill") COL_JOIN MENU_FUNC(menuBillReports) COL_JOIN \
    ARG1(MENU_PR_DATE, MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_HISTORY) MENU_MODE(MENU_MSUPER|MENU_MDAYFULL) MENU_NAME("Day FullBill") COL_JOIN MENU_FUNC(menuBillReports) COL_JOIN \
    ARG1(MENU_PR_DATE, MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  /* MENU_HIER(MENU_HIER_HISTORY) MENU_MODE(MENU_MSUPER|MENU_MDAY) MENU_NAME("DayBillPrned") COL_JOIN MENU_FUNC(menuDayBillPrned) COL_JOIN */ \
  /*  ARG1(MENU_PR_DATE, MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN */ \
  /* MENU_HIER(MENU_HIER_HISTORY) MENU_MODE(MENU_MSUPER|MENU_MDAY) MENU_NAME("Day Dup Bill") COL_JOIN MENU_FUNC(menuDayDupBill) COL_JOIN */ \
  /*  ARG1(MENU_PR_DATE, MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN */ \
  MENU_HIER(MENU_HIER_HISTORY) MENU_MODE(MENU_MSUPER|MENU_MDAYTAX|MENU_MMONTAX) MENU_NAME("Day Tax Repo") COL_JOIN MENU_FUNC(menuBillReports) COL_JOIN \
    ARG1(MENU_PR_DATE, MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_HISTORY) MENU_MODE(MENU_MSUPER|MENU_MMONITEM) MENU_NAME("Mon ItemBill") COL_JOIN MENU_FUNC(menuBillReports) COL_JOIN \
    ARG1(MENU_PR_MONTH , MENU_ITEM_MONTH) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_HISTORY) MENU_MODE(MENU_MSUPER|MENU_MMONFULL) MENU_NAME("Mon FullBill") COL_JOIN MENU_FUNC(menuBillReports) COL_JOIN \
    ARG1(MENU_PR_MONTH, MENU_ITEM_MONTH) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  /* MENU_HIER(MENU_HIER_HISTORY) MENU_MODE(MENU_MSUPER|MENU_MMONTH) MENU_NAME("MonBillPrned") COL_JOIN MENU_FUNC(menuMonBillPrned) COL_JOIN */ \
  /*  ARG1(MENU_PR_MONTH, MENU_ITEM_MONTH) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN */ \
  MENU_HIER(MENU_HIER_HISTORY) MENU_MODE(MENU_MSUPER|MENU_MMONTAX) MENU_NAME("Mon Tax Repo") COL_JOIN MENU_FUNC(menuBillReports) COL_JOIN \
    ARG1(MENU_PR_MONTH, MENU_ITEM_MONTH) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_HISTORY) MENU_MODE(MENU_MSUPER|MENU_MALLTAX) MENU_NAME("All ItemBill") COL_JOIN MENU_FUNC(menuBillReports) COL_JOIN \
    ARG1(MENU_PR_DATE, MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_DATE, MENU_ITEM_DATE) ROW_JOIN \
  MENU_HIER(MENU_HIER_HISTORY) MENU_MODE(MENU_MSUPER|MENU_MALLFULL) MENU_NAME("All FullBill") COL_JOIN MENU_FUNC(menuBillReports) COL_JOIN \
    ARG1(MENU_PR_DATE, MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_DATE, MENU_ITEM_DATE) ROW_JOIN \
  /* MENU_HIER(MENU_HIER_HISTORY) MENU_MODE(MENU_MSUPER) MENU_NAME("All Tax Repo") COL_JOIN MENU_FUNC(menuTaxReport) COL_JOIN */ \
  /*  ARG1(MENU_PR_DATE, MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_DATE, MENU_ITEM_DATE) ROW_JOIN */ \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER) MENU_NAME("Modify VAT  ") COL_JOIN MENU_FUNC(menuModVat) COL_JOIN \
    ARG1(MENU_PR_FLOAT, MENU_ITEM_FLOAT) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_SETTINGS) MENU_MODE(MENU_MSUPER) MENU_NAME("Mod Setting") COL_JOIN MENU_FUNC(menuSettingSet) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_HISTORY) MENU_MODE(MENU_MSUPER) MENU_NAME("Del All Bill") COL_JOIN MENU_FUNC(menuDelAllBill) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_SETTINGS) MENU_MODE(MENU_MSUPER|MENU_MNORMAL|MENU_MVALIDATE) MENU_NAME("Change Passw") COL_JOIN MENU_FUNC(menuSetPasswd) COL_JOIN \
    ARG1(MENU_PR_OLDPASS, MENU_ITEM_STR|MENU_ITEM_PASSWD) COL_JOIN ARG2(MENU_PR_PASS, MENU_ITEM_STR|MENU_ITEM_PASSWD) ROW_JOIN \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER) MENU_NAME("Set Serv Tax") COL_JOIN MENU_FUNC(menuSetServTax) COL_JOIN \
    ARG1(MENU_PR_FLOAT, MENU_ITEM_FLOAT) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_SETTINGS) MENU_MODE(MENU_MSUPER) MENU_NAME("Set DateTime") COL_JOIN MENU_FUNC(menuSetDateTime) COL_JOIN \
    ARG1(MENU_PR_DATE, MENU_ITEM_DATE) COL_JOIN ARG2(MENU_PR_TIME, MENU_ITEM_TIME) ROW_JOIN \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER)  MENU_NAME("Chg Usr,Pass") COL_JOIN MENU_FUNC(menuSetUserPasswd) COL_JOIN \
    ARG1(MENU_PR_NAME,  MENU_ITEM_STR) COL_JOIN ARG2(MENU_PR_PASS,  MENU_ITEM_STR|MENU_ITEM_PASSWD) ROW_JOIN \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER|MENU_MNORMAL)  MENU_NAME("User  Logout") COL_JOIN MENU_FUNC(menuUserLogout) COL_JOIN \
    ARG1(MENU_PR_ID,  MENU_ITEM_CONFIRM) COL_JOIN ARG2(MENU_PR_ID,  MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MRESET)  MENU_NAME("User Login  ") COL_JOIN MENU_FUNC(menuUserLogin) COL_JOIN \
    ARG1(MENU_PR_NAME,  MENU_ITEM_STR) COL_JOIN ARG2(MENU_PR_PASS,  MENU_ITEM_STR|MENU_ITEM_PASSWD) ROW_JOIN \
  MENU_HIER(MENU_HIER_SETTINGS) MENU_MODE(MENU_MSUPER|MENU_MRESET) MENU_NAME("Run Diagnost") COL_JOIN MENU_FUNC(menuRunDiag) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_SD) MENU_MODE(MENU_MSUPER) MENU_NAME("Load Items   ") COL_JOIN MENU_FUNC(menuSDLoadItem) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_SD) MENU_MODE(MENU_MSUPER) MENU_NAME("Export Items ") COL_JOIN MENU_FUNC(menuSDSaveItem) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_SD) MENU_MODE(MENU_MSUPER) MENU_NAME("Load Settings") COL_JOIN MENU_FUNC(menuSDLoadSettings) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_SD) MENU_MODE(MENU_MSUPER) MENU_NAME("ExportSetting") COL_JOIN MENU_FUNC(menuSDSaveSettings) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_SD) MENU_MODE(MENU_MSUPER|MENU_MNORMAL) MENU_NAME("Diag  Printer") COL_JOIN MENU_FUNC(menuDiagPrinter) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_DIAGNOSTIC) MENU_MODE(MENU_MSUPER|MENU_MNORMAL) MENU_NAME("PrintTestPage") COL_JOIN MENU_FUNC(menuPrintTestPage) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE)

extern uint8_t menu_error;
extern uint8_t bufSS[BUFSS_SIZE];

#define PSTR2STR(pstr, str, ui_1, ui_2)		\
  for (ui_2=0; ;ui_2++) {			\
    ui_1 = pgm_read_mem(pstr+ui_2);		\
    str[ui_2] = ui_1;				\
    if (0 == ui_1) break;			\
  }
  
  

/* Helper routines */
void menuInit(void);
void menuGetOpt(uint8_t *prompt, menu_arg_t *arg, uint8_t opt);
uint8_t menuGetChoice(uint8_t *quest, uint8_t *opt_arr, uint8_t choice_len, uint8_t max_idx);

/* User routines*/
void menuSetPasswd(uint8_t mode);
void menuSetUserPasswd(uint8_t mode);
void menuUserLogout(uint8_t mode);
void menuUserLogin(uint8_t mode);

/* Item routines */
void menuAddItem(uint8_t mode);
void menuDelItem(uint8_t mode);

/* billing routines */
void menuBilling(uint8_t mode);
void menuShowBill(uint8_t mode);

/* User option routines */
void menuSettingString(uint16_t addr, const char *quest, uint8_t max_chars);
void menuSettingUint32(uint16_t addr, const char *quest);
void menuSettingUint16(uint16_t addr, const char *quest);
void menuSettingUint8(uint16_t addr, const char *quest);
void menuSettingChoice(uint16_t addr, uint8_t *quest, uint8_t *opt_arr, uint8_t choice_len, const char *quest, uint8_t max_idx);

/* Report routines */
void menuPrnBill(struct sale *sl);
//void menuPrnFullBill(billing *bp);
void menuBillReports(uint8_t mode);
//void menuPrnItemBill(billing *bp);
//void menuPrnItemBillFooter(void);
//void menuPrnAllBill(billing *bp);
//void menuPrnTaxReport(billing *bp);
//void menuPrnTaxReportFooter(billing *bp);
void menuDelAllBill(uint8_t mode);

/* SD routines */
void menuSDLoadItem(uint8_t mode);
void menuSDSaveItem(uint8_t mode);
void menuSDLoadSettings(uint8_t mode);
void menuSDSaveSettings(uint8_t mode);

/* Other routines */
void menuRunDiag(uint8_t mode);
void menuMain(void);

#define menu_item_addr(id) ((id-1)*(uint16_t)ITEM_SIZEOF)

#define SD_ITEM_FILE      "items.dat"
#define SD_SETTINGS_FILE  "settings.dat"

#endif
