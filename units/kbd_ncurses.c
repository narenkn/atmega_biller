
#include <ncurses/ncurses.h>
#include "kdb.h"

volatile uint8_t KbdData;
volatile uint8_t KbdDataAvail;

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

void
KbdInit(void)
{
  cbreak();
  noecho();
}

void
KbdScan(void)
{
  assert(0);
}

#undef KBD_GETCH
#define KBD_GETCH				\
  KbdData = getch();				\
  KbdDataAvail = 1

uint8_t
KbdIsShiftPressed(void)
{
}
