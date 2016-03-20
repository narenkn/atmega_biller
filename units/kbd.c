
#include "kbd.h"

#ifndef  __UNITS_KBD_C
#define  __UNITS_KBD_C

keyHitData_t keyHitData;

uint16_t test_key_idx = -1;
uint8_t test_key_arr_idx = 0;
#define NUM_TEST_KEY_ARR 32
uint8_t *test_key[NUM_TEST_KEY_ARR];

#define RESET_TEST_KEYS KbdInit()
#define INIT_TEST_KEYS(A) do { test_key[test_key_arr_idx] = A; assert(test_key_arr_idx < NUM_TEST_KEY_ARR); test_key_arr_idx++; } while (0)
#define PRINT_TEST_KEYS(ui1) for (ui1=0; (ui1<NUM_TEST_KEY_ARR) && (0 != test_key[ui1]); ui1++) { printf("test_key[%d] = '%s', ", ui1, test_key[ui1]); } printf("\n")
#define KBD_KEY_TIMES(N)   ((N-1)<<4)
#define KBD_KEY(N)                  N

uint8_t KbdInTypeErrors = 1;

/* This should be kept in sync with original kbd.c */
const uint8_t keyChars[] = {
  /* KCHAR_ROWS x KCHAR_COLS */
  '0', ' ', '.', ',', ')', '+', '?', '_', ':',
  '1', 'a', 'b', 'c', '!', 'A', 'B', 'C', '~',
  '2', 'd', 'e', 'f', '@', 'D', 'E', 'F', '{',
  '3', 'g', 'h', 'i', '#', 'G', 'H', 'I', '}',
  '4', 'j', 'k', 'l', '$', 'J', 'K', 'L', '[',
  '5', 'm', 'n', 'o', '%', 'M', 'N', 'O', ']',
  '6', 'p', 'q', 'r', '^', 'P', 'Q', 'R', '|',
  '7', 's', 't', 'u', '&', 'S', 'T', 'U', '/',
  '8', 'v', 'w', 'x', '*', 'V', 'W', 'X', '<',
  '9', 'y', 'z', '(', '-', 'Y', 'Z', '=', '>',
};

void
KbdInit(void)
{
  uint8_t ui2;

  test_key_idx = 0; test_key_arr_idx = 0;

  for (ui2=0; ui2<NUM_TEST_KEY_ARR; ui2++)
    test_key[ui2] = NULL;
}

/* We make sure that if random char is introduced, a back
key is pressed and it is earsed. */
void
KbdGetCh(void)
{
  static uint8_t do_correct = 0;

  /* Previous key not yet consumed */
  if KBD_HIT
    return;

  /* All keys exhausted in latest pipe, look for pipeline */
  if (((uint16_t) -1) == test_key_idx) {
    if (test_key_arr_idx) {
      uint8_t ui2;
      for (ui2=0; ui2<(NUM_TEST_KEY_ARR-1); ui2++) {
	test_key[ui2] = test_key[ui2+1];
      }
      test_key[NUM_TEST_KEY_ARR-1] = NULL;
      test_key_arr_idx--;
      test_key_idx = 0;
    } else
      return;
  }
  assert(test_key_idx<=TEST_KEY_ARR_SIZE);

  if (NULL == test_key[0]) {
    assert(0);
    return;
  }

  /* last char of pipe */
  if ((0 == test_key[0][test_key_idx]) && (0 == do_correct)) {
    assert (test_key_idx >= 0);
    //printf("hack2 kbd.c sending:0x%x\n", keyHitData.KbdData);
    keyHitData.KbdData = ASCII_ENTER;
    keyHitData.KbdDataAvail = 1;
    test_key_idx = -1;
    return;
  }

  /* Certain times do not introduce typing errors */
  if ((0 == KbdInTypeErrors) ||
      (ASCII_ENTER==test_key[0][test_key_idx]) ||
      (ASCII_LEFT==test_key[0][test_key_idx]) ||
      (ASCII_RIGHT==test_key[0][test_key_idx])) {
    keyHitData.KbdData = test_key[0][test_key_idx];
    keyHitData.KbdDataAvail = 1;
    test_key_idx++;
    //printf("hack kbd.c sending:0x%x\n", keyHitData.KbdData);
    return;
  }

  keyHitData.KbdData = test_key[0][test_key_idx];
  keyHitData.KbdDataAvail = 1;
  test_key_idx++;
  if (2 <= do_correct) {
    do_correct = 0;
    //printf("Added correct key:'%c' '%s'\n", keyHitData.KbdData, test_key[0]);
  } else if (1 == do_correct) {
    keyHitData.KbdData = ASCII_LEFT;
    test_key_idx--;
    do_correct++;
    //printf("Added back key\n");
  } else if ((0 == do_correct) && (0 == (rand() % 2))) {
    keyHitData.KbdData = 'a' + (rand() % 26);
    //printf("Added random key: %c\n", keyHitData.KbdData);
    do_correct++;
    test_key_idx--;
  } else {
    //printf("Sending key: %c\n", test_key[0][test_key_idx]);
  }
  //  printf("do_correct:%d\n", do_correct);
}

uint8_t
KbdIsShiftPressed(void)
{
  uint8_t shift = 0;

  return shift;
}

/* Redefine certain macros for testing purpose */
#undef KBD_NOT_HIT
#define KBD_NOT_HIT (0)

#endif

#ifdef  INCL_UNITS_KBD_NCURSES_C

#include <ncurses/ncurses.h>
#include "kbd.h"

keyHitData_t keyHitData;

//extern PROGMEM uint8_t ps2code2ascii[];
//extern PROGMEM uint8_t ps2code2asciiE0[];

const uint8_t
keyChars[] PROGMEM = {
  /* KCHAR_ROWS x KCHAR_COLS */
  '0', ' ', '.', ',', ')', '+', '?', '_', ':',
  '1', 'a', 'b', 'c', '!', 'A', 'B', 'C', '~',
  '2', 'd', 'e', 'f', '@', 'D', 'E', 'F', '{',
  '3', 'g', 'h', 'i', '#', 'G', 'H', 'I', '}',
  '4', 'j', 'k', 'l', '$', 'J', 'K', 'L', '[',
  '5', 'm', 'n', 'o', '%', 'M', 'N', 'O', ']',
  '6', 'p', 'q', 'r', '^', 'P', 'Q', 'R', '|',
  '7', 's', 't', 'u', '&', 'S', 'T', 'U', '/',
  '8', 'v', 'w', 'x', '*', 'V', 'W', 'X', '<',
  '9', 'y', 'z', '(', '-', 'Y', 'Z', '=', '>',
};

/* ASCII_UNDEF in this table signifies good character */
uint8_t
cursesKbdFixes[] = {
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_BACKSPACE, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ASCII_UNDEF, ASCII_UNDEF, /*  0-15 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, /* 16-31 */
  ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, /* 32-47 */
  ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, /* 48-63 */
  ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, /* 64-79 */
  ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, /* 80-95 */
  ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, /* 96-111 */
  ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_DEFINED, ASCII_UNDEF, /* 112-127 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_BACKSPACE, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ASCII_UNDEF, ASCII_UNDEF, /*  128-143 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_BACKSPACE, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ASCII_UNDEF, ASCII_UNDEF, /*  144-159 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_BACKSPACE, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ASCII_UNDEF, ASCII_UNDEF, /*  160-175 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_BACKSPACE, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ASCII_UNDEF, ASCII_UNDEF, /*  176-191 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_BACKSPACE, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ASCII_UNDEF, ASCII_UNDEF, /*  192-207 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_BACKSPACE, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ASCII_UNDEF, ASCII_UNDEF, /*  208-223 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_BACKSPACE, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ASCII_UNDEF, ASCII_UNDEF, /*  224-239 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_BACKSPACE, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ASCII_UNDEF, ASCII_UNDEF, /*  240-255 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_BACKSPACE, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ASCII_UNDEF, ASCII_UNDEF, /*  256-271 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_BACKSPACE, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ASCII_UNDEF, ASCII_UNDEF, /*  272-287 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_BACKSPACE, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ASCII_UNDEF, ASCII_UNDEF, /*  288-303 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_BACKSPACE, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ASCII_UNDEF, ASCII_UNDEF, /*  304-319 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_BACKSPACE, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ASCII_UNDEF, ASCII_UNDEF, /*  320-335 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_BACKSPACE, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ASCII_UNDEF, ASCII_UNDEF, /*  336-351 */
};

void
KbdInit(void)
{
  cbreak();
  noecho();
  cursesKbdFixes[KEY_UP] = ASCII_UP;
  cursesKbdFixes[KEY_LEFT] = ASCII_LEFT;
  cursesKbdFixes[KEY_RIGHT] = ASCII_RIGHT;
  cursesKbdFixes[KEY_DOWN] = ASCII_DOWN;
  cursesKbdFixes[KEY_DC] = ASCII_DEL;
  cursesKbdFixes[KEY_DL] = ASCII_DEL;
  cursesKbdFixes[KEY_F(2)] = ASCII_F2;
  cursesKbdFixes[KEY_PRINT] = ASCII_PRNSCRN;
  cursesKbdFixes[KEY_F(4)] = ASCII_NUMLK;
  cursesKbdFixes[KEY_BACKSPACE] = ASCII_BACKSPACE;
  cursesKbdFixes[KEY_ENTER] = ASCII_ENTER;
  cursesKbdFixes[ASCII_LF] = ASCII_ENTER;
  cursesKbdFixes[ASCII_ENTER] = ASCII_ENTER;
  cursesKbdFixes[27] = ASCII_ESCAPE;
}

void
KbdScan(void)
{
  assert(0);
}

#undef KBD_GETCH
#define KBD_GETCH	do {						\
    uint16_t  t_data;							\
    keyHitData.KbdDataAvail = 0;					\
    t_data = getch();							\
    assert(t_data < sizeof(cursesKbdFixes));				\
    /*move(1, 0);*/							\
    /*printw("kbd raw1:0x%x    ", t_data);*/				\
    if (ASCII_UNDEF == cursesKbdFixes[t_data]) {			\
    } else if (ASCII_DEFINED == cursesKbdFixes[t_data]) {		\
      keyHitData.KbdData = t_data;					\
      assert(keyHitData.KbdData == t_data); /* check data size */	\
      keyHitData.KbdDataAvail = 1;					\
    } else {								\
      keyHitData.KbdData = cursesKbdFixes[t_data];			\
      keyHitData.KbdDataAvail = 1;					\
    }									\
    /*move(2, 0);*/							\
    /*printw("kbd raw2:0x%x    ", keyHitData.KbdData);*/		\
  } while (0 == keyHitData.KbdDataAvail)

uint8_t
KbdIsShiftPressed(void)
{
}

#endif

