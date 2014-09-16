#ifndef AVR_BOOT_H
#define AVR_BOOT_H

/* Flash size
   64K / SPM_PAGESIZE
 */
uint8_t _flash[64*(1024/SPM_PAGESIZE)][SPM_PAGESIZE];
uint8_t _flash_page[SPM_PAGESIZE];

void
boot_page_erase(uint16_t page)
{
uint16_t ui1;
assert(page < (64*(1024/SPM_PAGESIZE)));
for (ui1=0; ui1<SPM_PAGESIZE; ui1++)
  _flash[page][ui1] = 0xFF;
}

#define boot_spm_busy_wait()

#define boot_rww_enable()

void
boot_page_write(uint16_t page)
{
  uint16_t ui1;
  for (ui1=0; ui1<SPM_PAGESIZE; ui1++) {
    _flash[page][ui1] = _flash_page[ui1];
  }
}

void
boot_page_fill(uint8_t *addr, uint8_t val)
{
  uint16_t page = (uint16_t)addr;
  assert((page>>FLASH_PAGE_SIZE_LOGN) < 64*(1024/SPM_PAGESIZE));
  _flash[page>>FLASH_PAGE_SIZE_LOGN][page&(SPM_PAGESIZE-1)] = val;
}

#endif
