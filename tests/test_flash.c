#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <avr/eeprom.h>
#include <stdlib.h>

#include "lcd.c"

uint8_t buf[SPM_PAGESIZE];

/*---------------------------------------------------------*/
/* User Provided RTC Function called by FatFs module       */
uint32_t
get_fattime(void)
{
  /* Returns current time packed into a uint32_t variable */
  return	  ((uint32_t)(2014 - 1980) << 25)	/* Year 2013 */
    | ((uint32_t)5 << 21)				/* Month 7 */
    | ((uint32_t)6 << 16)				/* Mday 28 */
    | ((uint32_t)1 << 11)				/* Hour 0 */
    | ((uint32_t)19 << 5)				/* Min 0 */
    | ((uint32_t)44 >> 1);				/* Sec 0 */
}

uint16_t testBuf[SPM_PAGESIZE/sizeof(uint16_t)];
uint16_t testLoc[SPM_PAGESIZE] PROGMEM =
  { [ 0 ... (SPM_PAGESIZE - 1) ] = 0 };

void boot_program_page() BOOTLOADER_SECTION;

const uint8_t data[8] PROGMEM = {0x1, 0x3, 0x5, 0x8, 0x13, 0x21, 0x34, 0x55};

int
main()
{
  LCD_init();

  DDRD |= 0x10 ; LCD_bl_on;
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("Flash test:"));
  _delay_ms(1);

  boot_program_page();

  LCD_PUTCH('@');

  /* don't come back */
  for (;;) { }
}

void
boot_program_page()
{
  uint16_t ui16_1, ui16_2, ui16_3, rand_seed;
  uint8_t sreg;
  uint32_t page = ((uint32_t)testLoc) & ~((uint32_t)(SPM_PAGESIZE-1));

  /* init */
  rand_seed = get_fattime();

  /* Disable interrupts */
  sreg = SREG;
  cli();

  eeprom_busy_wait ();

  boot_page_erase (page);
  boot_spm_busy_wait ();      // Wait until the memory is erased.

  /* randomize */
  srand(rand_seed);
  for (ui16_1=0; ui16_1<SPM_PAGESIZE; ui16_1++) {
    testBuf[ui16_1] = rand();
  }

  /* write to flash */
  for (ui16_1=0; ui16_1<SPM_PAGESIZE; ui16_1+=sizeof(uint16_t)) {
    boot_page_fill (page+ui16_1, testBuf[ui16_1]);
  }

  boot_page_write (page);     // Store buffer in flash page.
  boot_spm_busy_wait();       // Wait until the memory is written.

  // Reenable RWW-section again. We need this if we want to jump back
  // to the application after bootloading.
  boot_rww_enable ();

  // Re-enable interrupts (if they were ever enabled).
  SREG = sreg;

  /* read back and verify */
  srand(rand_seed);
  ui16_3 = 0;
  for (ui16_1=0; ui16_1<SPM_PAGESIZE;ui16_1+=sizeof(uint16_t)) {
    ui16_2 = pgm_read_word(page+ui16_1);
    if (ui16_2 != testBuf[ui16_1])
      ui16_3++;
  }

  /* report */
  LCD_CLRLINE(1);
  LCD_WR_P(PSTR("#errors:"));
  LCD_PUT_UINT16X(ui16_3);
}
