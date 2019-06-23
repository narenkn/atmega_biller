#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <main.h>

#include "lcd.c"
#include "i2c.c"
#include "kbd.c"
#include "spi.c"
#include "flash.c"
#include "main.c"

uint8_t buf1[NVF_PAGE_SIZE], buf2[NVF_PAGE_SIZE];

volatile uint8_t menuPendActs = 0;
void disk_timerproc() {}

int
main(void) 
{
  uint16_t errors = 0;

  LCD_init();
  LCD_bl_on;
  spiInit();
  nvfInit();
  main_init();

  sei();

  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("Test Small R/W"));
  bill_write_bytes(0, (uint8_t *)"Hello World", 11);
  bill_read_bytes(0, buf1, 11);
  LCD_CLRLINE(1);
  LCD_WR_N(buf1, 11);
  _delay_ms(1000);

  /* init randomizer */
  uint32_t time = get_fattime();
  srand(time);

  /* check many sectors */
  uint16_t loop, loop_start = 10;
  uint16_t sector;
  for (loop=loop_start; loop; loop--) {

    /* randomly choose a rand sector */
    sector = rand() & NVF_END_ADDRESS;
    errors = 0;

    /* randomize flash area */
    for (uint16_t ui1=NVF_PAGE_SIZE; ui1; ui1--) {
      buf1[ui1] = rand();
    }
    bill_write_bytes(sector, buf1, NVF_PAGE_SIZE);

    /* read back and check */
    bill_read_bytes(sector, buf2, NVF_PAGE_SIZE);
    for (uint16_t ui1=NVF_PAGE_SIZE; ui1; ui1--) {
      if (buf1[ui1-1] != buf2[ui1-1])
	errors++;
    }

    if (errors) break;
  }

  /* Result */
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("LoopSt:"));
  LCD_PUT_UINT(loop_start);
  LCD_WR_P(PSTR("Loop:"));
  LCD_PUT_UINT(loop);
  LCD_CLRLINE(1);
  LCD_WR_P(PSTR("S:"));
  LCD_PUT_UINT16X(sector);
  LCD_WR_P(PSTR(" Err:"));
  LCD_PUT_UINT(errors);
  _delay_ms(1000);

#if 0
  /* Test search
     if name == 16 bytes,
       256 bytes => 16 names (2^4)
       1K => 64 (2^6)
       100K => 6400.
     Need to test this size search!!
   */
  uint32_t time_before = get_fattime();
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("Test Search"));
  uint16_t numSectors = 1<<12; /* 100K / 256 => 4K sectors */
  errors=0;
  for (uint16_t sector=0; numSectors; numSectors--, sector++) {
    bill_read_bytes(sector, buf2, NVF_PAGE_SIZE);
    for (uint16_t idx=NVF_PAGE_SIZE; idx; idx--)
      errors += (buf1[idx] == buf2[idx]);
  }
  uint32_t time_after = get_fattime();
  LCD_CLRLINE(1);
  LCD_PUT_UINT(time_before);
  LCD_PUTCH(' ');
  LCD_PUT_UINT(time_after);
#endif

  /* stuck forever */
  while (1) {};

  return 0;
} 