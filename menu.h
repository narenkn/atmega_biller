#ifndef MENU_H
#define MENU_H

/* Documentation
   No. of users : EPS_MAX_USERS
   Super user   : 'admin'
 */


/* Type of Menu argument, conveniently spaced
   DATE & MONTH apart (used as item_type)
 */
#define MENU_ITEM_NONE                 0
#define MENU_ITEM_ID                   1
#define MENU_ITEM_DATE                 2
#define MENU_ITEM_STR                  3
#define MENU_ITEM_MONTH                4
#define MENU_ITEM_FLOAT                5
#define MENU_ITEM_TIME                 6
#define MENU_ITEM_TABLE                7
#define MENU_ITEM_TYPE_MASK         0x1F
#define MENU_ITEM_OPTIONAL         (1<<7)
#define MENU_ITEM_DONTCARE_ON_PREV (1<<6)
#define MENU_ITEM_PASSWD           (1<<5)

/* Type of Menu Error */
#define MENU_RET_NOERROR               0
#define MENU_RET_NOTAGAIN          (1<<0)
#define MENU_RET_CALLED            (1<<2)

/* Prompt for Menu argument */
#define MENU_PROMPT_LEN 8
#define MENU_PROMPTS	\
  "None    " /* 0 */    \
  "Id      " /* 1 */    \
  "Name    " /* 2 */    \
  "OldPaswd" /* 3 */    \
  "Password" /* 4 */    \
  "Month   " /* 5 */    \
  "TimeHHMM" /* 6 */    \
  "Table   " /* 7 */    \
  "DDMMYYYY" /* 8 */    \
  "FrDDMMYY" /* 9 */    \
  "ToDDMMYY" /*10 */    \
  "Decimal " /*11 */

#define MENU_PR_NONE         0
#define MENU_PR_ID           1
#define MENU_PR_NAME         2
#define MENU_PR_PASS         3
#define MENU_PR_MONTH        4
#define MENU_PR_TIME         5
#define MENU_PR_DATE         6
#define MENU_PR_FROM_DATE    7
#define MENU_PR_TO_DATE      8
#define MENU_PR_FLOAT        9

typedef struct {
  union {
    struct {
      /* Total 24 bit max */
      uint8_t  i8;  /* msb */
      uint16_t i16;
      uint32_t i32;
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
    struct {
      uint16_t len;
      uint8_t *sptr;
    } str;
  } value;
  uint8_t   valid;
} menu_arg_t;

#define MENU_NAMES_LEN 16

/* One super-user and other billing users */
#define MENU_MREDOCALL   0x10  /* Continuous operation until exit */
#define MENU_MSUPER      0x20
#define MENU_MNORMAL     0x40  /* User functions */
#define MENU_MRESET      0x80  /* State after reset */
#define MENU_MODITEM     0x01
#define MENU_MVALIDATE   0x02
#define MENU_NOCONFIRM   0x03
#define MENU_KOTBILL     0x04
#define MENU_VOIDBILL    0x05
#define MENU_REPO_BWISE  0x01
#define MENU_REPO_VOID   0x02
#define MENU_REPO_DUP    0x03
#define MENU_REPO_TAX    0x04
#define MENU_REPO_ITWISE 0x05
#define MENU_REPO_ITEM   0x06
#define MENU_MODEMASK    0xE0

/* Implementation of Hierarchical menu : Every menu item could be associated
   with a main menu */
#define MENU_HIER_BILLING    1
#define MENU_HIER_SETTINGS   2
#define MENU_HIER_REPORTS    3
#define MENU_HIER_MAX        3
#define MENU_HIER_NAME_SIZE  7
#define MENU_HIER_NAMES      "Billing" \
                             "Setting" \
                             "Reports"

#define SETTING_VAR_NAME_LEN    12
struct setting_vars {
  uint8_t type;
  uint8_t size;
  uint8_t size2;
  uint8_t name[SETTING_VAR_NAME_LEN];
  uint16_t ep_ptr;
};

#define MENU_ITEMS \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MRESET)  MENU_NAME("User Login      ") COL_JOIN MENU_FUNC(menuUserLogin) COL_JOIN \
    ARG1(MENU_PR_NAME,  MENU_ITEM_STR) COL_JOIN ARG2(MENU_PR_PASS,  MENU_ITEM_STR|MENU_ITEM_PASSWD) ROW_JOIN \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER|MENU_MNORMAL)  MENU_NAME("User  Logout    ") COL_JOIN MENU_FUNC(menuUserLogout) COL_JOIN \
    ARG1(MENU_PR_ID,  MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID,  MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER|MENU_MNORMAL|MENU_MREDOCALL)  MENU_NAME("New Bill        ") COL_JOIN MENU_FUNC(menuBilling) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_TABLE|MENU_ITEM_OPTIONAL) ROW_JOIN \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER|MENU_MNORMAL|MENU_MREDOCALL|MENU_KOTBILL)  MENU_NAME("New KOT Bill    ") COL_JOIN MENU_FUNC(menuBilling) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_TABLE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER|MENU_MNORMAL|MENU_MREDOCALL|MENU_VOIDBILL)  MENU_NAME("Reopen Void Bill ") COL_JOIN MENU_FUNC(menuBilling) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_ID|MENU_ITEM_OPTIONAL) ROW_JOIN \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER|MENU_MNORMAL|MENU_MREDOCALL)  MENU_NAME("Calculator      ") COL_JOIN MENU_FUNC(menuCalculator) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_TABLE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER|MENU_MNORMAL|MENU_MODITEM)  MENU_NAME("Modify Bill     ") COL_JOIN MENU_FUNC(menuBilling) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_ID|MENU_ITEM_OPTIONAL) ROW_JOIN \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER) MENU_NAME("Add Item        ") COL_JOIN MENU_FUNC(menuAddItem) COL_JOIN \
    ARG1(MENU_PR_NAME, MENU_ITEM_STR|MENU_ITEM_OPTIONAL) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_ID|MENU_ITEM_OPTIONAL) ROW_JOIN \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER) MENU_NAME("Delete Item     ") COL_JOIN MENU_FUNC(menuDelItem) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_ID) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER|MENU_MODITEM) MENU_NAME("Modify Item     ") COL_JOIN MENU_FUNC(menuAddItem) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_ID) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER|MENU_MNORMAL)  MENU_NAME("Show Bill       ") COL_JOIN MENU_FUNC(menuShowBill) COL_JOIN \
    ARG1(MENU_PR_FROM_DATE,  MENU_ITEM_DATE|MENU_ITEM_OPTIONAL) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_ID|MENU_ITEM_OPTIONAL) ROW_JOIN \
  MENU_HIER(MENU_HIER_BILLING) MENU_MODE(MENU_MSUPER|MENU_MNORMAL)  MENU_NAME("Tally Cash      ") COL_JOIN MENU_FUNC(menuTallyCash) COL_JOIN \
    ARG1(MENU_PR_NONE,  MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_NONE, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_REPORTS) MENU_MODE(MENU_MSUPER|MENU_REPO_BWISE) MENU_NAME("Bill Wise Report") COL_JOIN MENU_FUNC(menuBillReports) COL_JOIN \
    ARG1(MENU_PR_FROM_DATE, MENU_ITEM_DATE|MENU_ITEM_OPTIONAL) COL_JOIN ARG2(MENU_PR_TO_DATE, MENU_ITEM_DATE|MENU_ITEM_OPTIONAL) ROW_JOIN \
  MENU_HIER(MENU_HIER_REPORTS) MENU_MODE(MENU_MSUPER|MENU_REPO_VOID) MENU_NAME("Void Report     ") COL_JOIN MENU_FUNC(menuBillReports) COL_JOIN \
    ARG1(MENU_PR_FROM_DATE, MENU_ITEM_DATE|MENU_ITEM_OPTIONAL) COL_JOIN ARG2(MENU_PR_TO_DATE, MENU_ITEM_DATE|MENU_ITEM_OPTIONAL) ROW_JOIN \
  MENU_HIER(MENU_HIER_REPORTS) MENU_MODE(MENU_MSUPER|MENU_REPO_DUP) MENU_NAME("Duplicte BillRpt") COL_JOIN MENU_FUNC(menuBillReports) COL_JOIN \
    ARG1(MENU_PR_FROM_DATE, MENU_ITEM_DATE|MENU_ITEM_OPTIONAL) COL_JOIN ARG2(MENU_PR_TO_DATE, MENU_ITEM_DATE|MENU_ITEM_OPTIONAL) ROW_JOIN \
  MENU_HIER(MENU_HIER_REPORTS) MENU_MODE(MENU_MSUPER|MENU_MTAX) MENU_NAME("Tax Report      ") COL_JOIN MENU_FUNC(menuBillReports) COL_JOIN \
    ARG1(MENU_PR_FROM_DATE, MENU_ITEM_DATE|MENU_ITEM_OPTIONAL) COL_JOIN ARG2(MENU_PR_TO_DATE, MENU_ITEM_DATE|MENU_ITEM_OPTIONAL) ROW_JOIN \
  MENU_HIER(MENU_HIER_REPORTS) MENU_MODE(MENU_MSUPER|MENU_REPO_ITEM) MENU_NAME("Item In Bill Rept") COL_JOIN MENU_FUNC(menuBillReports) COL_JOIN \
    ARG1(MENU_PR_FROM_DATE, MENU_ITEM_DATE|MENU_ITEM_OPTIONAL) COL_JOIN ARG2(MENU_PR_TO_DATE, MENU_ITEM_DATE|MENU_ITEM_OPTIONAL) ROW_JOIN \
  MENU_HIER(MENU_HIER_REPORTS) MENU_MODE(MENU_MSUPER) MENU_NAME("Delete All Bill ") COL_JOIN MENU_FUNC(menuDelAllBill) COL_JOIN \
    ARG1(MENU_PR_DATE, MENU_ITEM_DATE|MENU_ITEM_OPTIONAL) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_ID|MENU_ITEM_OPTIONAL) ROW_JOIN \
  MENU_HIER(MENU_HIER_SETTINGS) MENU_MODE(MENU_MSUPER) MENU_NAME("Modify Setting  ") COL_JOIN MENU_FUNC(menuSettingSet) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_SETTINGS) MENU_MODE(MENU_MSUPER) MENU_NAME("Set Hot Key     ") COL_JOIN MENU_FUNC(menuSetHotKey) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_SETTINGS) MENU_MODE(MENU_MSUPER|MENU_MNORMAL|MENU_MVALIDATE) MENU_NAME("Change Password ") COL_JOIN MENU_FUNC(menuSetPasswd) COL_JOIN \
    ARG1(MENU_PR_OLDPASS, MENU_ITEM_STR|MENU_ITEM_PASSWD) COL_JOIN ARG2(MENU_PR_PASS, MENU_ITEM_STR|MENU_ITEM_PASSWD) ROW_JOIN \
  MENU_HIER(MENU_HIER_SETTINGS) MENU_MODE(MENU_MSUPER)  MENU_NAME("Change Usr, Pass") COL_JOIN MENU_FUNC(menuSetUserPasswd) COL_JOIN \
    ARG1(MENU_PR_NAME,  MENU_ITEM_STR) COL_JOIN ARG2(MENU_PR_PASS,  MENU_ITEM_STR|MENU_ITEM_PASSWD) ROW_JOIN \
  MENU_HIER(MENU_HIER_SETTINGS) MENU_MODE(MENU_MSUPER) MENU_NAME("Set Date,Time   ") COL_JOIN MENU_FUNC(menuSetDateTime) COL_JOIN \
    ARG1(MENU_PR_DATE, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_TIME, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_SETTINGS) MENU_MODE(MENU_MSUPER|MENU_MNORMAL) MENU_NAME("Run Diagnostic  ") COL_JOIN MENU_FUNC(menuRunDiag) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_SETTINGS) MENU_MODE(MENU_MSUPER) MENU_NAME("Factory Reset   ") COL_JOIN MENU_FUNC(menuFactorySettings) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_SETTINGS) MENU_MODE(MENU_MSUPER) MENU_NAME("Load Items      ") COL_JOIN MENU_FUNC(menuSDLoadItem) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_SETTINGS) MENU_MODE(MENU_MSUPER) MENU_NAME("Export Items    ") COL_JOIN MENU_FUNC(menuSDSaveItem) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_SETTINGS) MENU_MODE(MENU_MSUPER) MENU_NAME("Load Setting    ") COL_JOIN MENU_FUNC(menuSDLoadSettings) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) ROW_JOIN \
  MENU_HIER(MENU_HIER_SETTINGS) MENU_MODE(MENU_MSUPER) MENU_NAME("Export Setting  ") COL_JOIN MENU_FUNC(menuSDSaveSettings) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE) 
  MENU_HIER(MENU_HIER_SETTINGS) MENU_MODE(MENU_MSUPER) MENU_NAME("Update Firmware  ") COL_JOIN MENU_FUNC(menuUpdateFirmware) COL_JOIN \
    ARG1(MENU_PR_ID, MENU_ITEM_NONE) COL_JOIN ARG2(MENU_PR_ID, MENU_ITEM_NONE)

extern uint8_t menu_error;
extern uint8_t bufSS[BUFSS_SIZE];

/* Device status */
extern uint8_t devStatus;
#define DS_NO_SD    (1<<0)
#define DS_NO_TFT   (1<<1)
#define DS_DEV_1K   (1<<2)
#define DS_DEV_5K   (1<<3)
#define DS_DEV_20K  (1<<4)
#define DS_DEV_INVALID (1<<5)
#define DS_DEV_ERROR (DS_DEV_INVALID|DS_NO_TFT|DS_NO_SD)

/* Pending actions */
#define MENU_PEND_LCD_REFRESH  (1<<0)
#define MENU_PEND_NEW_DAY      (1<<1)
extern uint8_t menuPendActs;
void menuPendNewDay(void);

/* Helper routines */
typedef uint32_t (*menuGetOptHelper)(uint8_t *str, uint16_t strlen, uint32_t prev);
void menuInit(void);
void menuGetOpt(const uint8_t *prompt, menu_arg_t *arg, uint8_t opt, menuGetOptHelper helper);
uint8_t menuGetChoice(const uint8_t *quest, uint8_t *opt_arr, uint8_t choice_len, uint8_t max_idx);
uint8_t menuGetYesNo(const uint8_t *quest, uint8_t size);
uint32_t menuFloatPercentHelper(uint8_t *str, uint16_t strlen, uint32_t prev); // Unverified
void menuScanF(char *str, uint32_t *ui32);
#define SSCANF(str_p, ui32)			\
  *(ui32) = 0;					\
  menuScanFDotSeen = 0;				\
  menuScanF(str_p, &ui32);			\
  if (menuScanFDotSeen < 2) ui32 *= 10;		\
  if (menuScanFDotSeen < 3) ui32 *= 10
extern uint8_t menuScanFDotSeen;

/* User routines*/
uint8_t menuSetPasswd(uint8_t mode);
uint8_t menuSetUserPasswd(uint8_t mode);
uint8_t menuUserLogout(uint8_t mode);
uint8_t menuUserLogin(uint8_t mode);

/* Item routines */
uint8_t menuAddItem(uint8_t mode); // Unverified
uint8_t menuDelItem(uint8_t mode); // Unverified
void menuIndexItem(struct item *it); // Unverified
uint16_t menuItemFind(uint8_t *name, uint8_t *prod_code, struct item *it, uint16_t idx); // Unverified

/* billing routines */
uint8_t menuVoidBilling(uint8_t mode); // unverified
uint8_t menuBilling(uint8_t mode); // unverified
uint8_t menuCalculator(uint8_t mode); // unverified
uint8_t menuShowBill(uint8_t mode); // unverified
typedef void (*menuPrnBillItemHelper)(uint16_t item_id, struct item *it, uint16_t it_index); // unverified
void menuPrnBillEE24xxHelper(uint16_t item_id, struct item *it, uint16_t it_index); // unverified
void menuPrnD(uint32_t var);
void menuPrnF(uint32_t var);
void menuPrnBill(struct sale *sl, menuPrnBillItemHelper nitem); // Unverified

/* User option routines */
#if MENU_SETTING_ENABLE
void menuSettingString(uint16_t addr, const uint8_t *quest, uint16_t max_chars);
void menuSettingUint32(uint16_t addr, const uint8_t *quest);
void menuSettingUint16(uint16_t addr, const uint8_t *quest);
void menuSettingUint8(uint16_t addr, const uint8_t *quest);
void menuSettingBit(uint16_t addr, const uint8_t *quest, uint8_t size, uint8_t offset);
#endif
uint8_t menuSetDateTime(uint8_t mode);
uint8_t menuSettingSet(uint8_t mode);
uint8_t menuSetHotKey(uint8_t mode); // Unverified

/* Report routines */
uint8_t menuBillReports(uint8_t mode); // Unverified
uint8_t menuDelAllBill(uint8_t mode); // Unverified

/* SD routines */
uint8_t menuSDLoadItem(uint8_t mode); // Unverified
uint8_t menuSDLoadSettings(uint8_t mode); // Unverified
uint8_t menuSDSaveItem(uint8_t mode); // Unverified
uint8_t menuSDSaveSettings(uint8_t mode); // Unverified

/* Other routines */
uint8_t menuFactorySettings(uint8_t mode); // Unverified
uint8_t menuUpdateFirmware(uint8_t mode); // Unverified
uint8_t menuRunDiag(uint8_t mode); // Unverified
void menuMain(void);

extern  uint16_t         diagStatus;
#define DIAG_LCD              (1<<0)
#define DIAG_DISP_BRIGHT      (1<<1)
#define DIAG_PRINTER          (1<<2)
#define DIAG_MEM1             (1<<3)
#define DIAG_MEM3             (1<<4)
#define DIAG_TIMER            (1<<5)
#define DIAG_KEYPAD           (1<<6)
#define DIAG_WEIGHING_MC      (1<<7)
#define DIAG_BUZZER           (1<<8)

#ifndef SD_BILLING_FILE
#define SD_BILLING_FILE       "billing.dat"
#endif

#endif
