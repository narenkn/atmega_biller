#define NVF_CHIP_SIZE (1<<23) /* 8M bytes = 32Mbits */
uint8_t _nvf_data[NVF_CHIP_SIZE/NVF_PAGE_SIZE][NVF_PAGE_SIZE];
              /* [1^15] [1^8] */
uint8_t _nvf_sleep = 0;

bool
nvfInit()
{
  assert(0 == _nvf_sleep);

  memset(_nvf_data, 0xFF, NVF_CHIP_SIZE);

  return true;
}

void
nvfChipErase(uint8_t sel)
{
  assert(0 == _nvf_sleep);

  memset(_nvf_data, 0xFF, NVF_CHIP_SIZE);
}

/* addr is 4k addr */
void
nvfBlockErase4K(uint16_t addr)
{
  assert(0 == _nvf_sleep);
  addr <<= 1;

  for (uint16_t ui16_1=0; ui16_1<16; ui16_1++, addr++) {
    memset(((uint8_t *)_nvf_data)+(addr*NVF_PAGE_SIZE), 0xFF, NVF_PAGE_SIZE);
  }
}

void
nvfSleep()
{
  _nvf_sleep = 1;
}

void
nvfWakeUp()
{
  _nvf_sleep = 0;
}

/* addr : is always 512 blocks aligned
 */
uint16_t
bill_write_bytes(uint16_t addr, uint8_t* buf, uint16_t len)
{
  assert(0 == _nvf_sleep);

  uint8_t  ui8_1;
  uint16_t ui16_1;
  uint8_t* lbuf = buf;
  addr <<= 1;

  for (ui8_1=0, ui16_1=0; len; len--, ui8_1++, ui16_1++) {
    _nvf_data[addr][ui8_1] = (NULL == buf) ? 0 : *lbuf++;
    addr += (0xFF == ui8_1) ? 1 : 0;
  }

  return ui16_1;
}

uint16_t
bill_read_bytes(uint16_t addr, uint8_t* buf, uint16_t len)
{
  assert(0 == _nvf_sleep);
  assert(buf);

  uint8_t  ui8_1;
  uint16_t ui16_1;
  addr <<= 1;

  for (ui8_1=0, ui16_1=0; len; len--, ui8_1++, ui16_1++, buf++) {
    buf[0] = _nvf_data[addr][ui8_1];
    addr += (0xFF == ui8_1) ? 1 : 0;
  }

  return ui16_1;
}
