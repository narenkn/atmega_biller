
#include "kbd.h"

uint8_t KbdData;
uint8_t KbdDataAvail;

uint16_t test_key_idx = -1;
uint8_t test_key_arr_idx = 0;
#define NUM_TEST_KEY_ARR 16
uint8_t *test_key[NUM_TEST_KEY_ARR];

#define INIT_TEST_KEYS(A) do { test_key[test_key_arr_idx+1] = A; test_key_arr_idx++; assert((test_key_arr_idx+1) < NUM_TEST_KEY_ARR); } while (0)
#define KBD_KEY_TIMES(N)   ((N-1)<<4)
#define KBD_KEY(N)                  N

/* This should be kept in sync with original kbd.c */
__code const uint8_t keyChars[] = {
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

  for (ui2=0; ui2<NUM_TEST_KEY_ARR; ui2++)
    test_key[ui2] = NULL;
}

/* FIXME: We make sure that if random char is introduced, a back
key is pressed and it is earsed. Testing is not being done when
the last key is a 'back key' */
void
KbdGetCh(void)
{
  static uint8_t do_correct = 0;

  if KBD_HIT
    return;
  if (((uint16_t) -1) == test_key_idx) {
    if (test_key_arr_idx) {
      uint8_t ui2;
      for (ui2=0; ui2<(NUM_TEST_KEY_ARR-1); ui2++) {
	test_key[ui2] = test_key[ui2+1];
      }
      test_key[NUM_TEST_KEY_ARR-1] = NULL;
      test_key_arr_idx--;
    } else
      return;
  }
  assert(test_key_idx<=FLASH_SECTOR_SIZE);
  if ((0 == test_key[0][test_key_idx]) && (0 == do_correct)) { /* completed */
    assert (((uint16_t)-1) != test_key_idx);
    KbdData = ASCII_ENTER;
    KbdDataAvail = 1;
    test_key_idx = -1;
    //    printf("hack2 kbd.c sending:0x%x\n", KbdData);
    return;
  }

  if ((ASCII_ENTER==test_key[0][test_key_idx]) || (ASCII_LEFT==test_key[0][test_key_idx]) || (ASCII_RIGHT==test_key[0][test_key_idx])) {
    KbdData = test_key[0][test_key_idx];
    KbdDataAvail = 1;
    test_key_idx++;
    //    printf("hack kbd.c sending:0x%x\n", KbdData);
    return;
  }

  KbdData = test_key[0][test_key_idx];
  KbdDataAvail = 1;
  test_key_idx++;
  if (2 <= do_correct) {
    do_correct = 0;
    //    printf("Added correct key\n");
  } else if (1 == do_correct) {
    KbdData = ASCII_LEFT;
    test_key_idx--;
    do_correct++;
    //    printf("Added back key\n");
  } else if ((0 == do_correct) && (0 == (rand() % 2))) {
    KbdData = 'a' + (rand() % 26);
    //    printf("Added random key: %c\n", KbdData);
    do_correct++;
    test_key_idx--;
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
