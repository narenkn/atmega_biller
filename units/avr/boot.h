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
  for (ui1=SPM_PAGESIZE; ui1!=1; ui1>>=1) {
    page >>= 1;
  }
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
  for (ui1=SPM_PAGESIZE; ui1!=1; ui1>>=1) {
    page >>= 1;
  }
  for (ui1=0; ui1<SPM_PAGESIZE; ui1++) {
    _flash[page][ui1] = _flash_page[ui1];
  }
}

void
boot_page_fill(uint8_t *addr, uint16_t val)
{
  uint16_t ui1;
  uint16_t page = (uint16_t)addr;
  for (ui1=SPM_PAGESIZE; ui1!=1; ui1>>=1) {
    page>>=1;
  }
  ui1 = (uint16_t)addr;
  assert(page < 64*(1024/SPM_PAGESIZE));
  _flash[page][ui1&(SPM_PAGESIZE-1)] = val&0xFF;
  _flash[page][(ui1&(SPM_PAGESIZE-1)) ^ 1] = (val>>8)&0xFF;

  printf("addr:%p = 0x%x\n", addr, val);
}

#endif
