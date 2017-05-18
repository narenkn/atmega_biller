/* addr : is always 512 blocks aligned
   addr[15:14] selects device
   addr[13:0] corresponds to [22:9] (incoming block size: 512)
 */
#define NVF_CHIP_SIZE (1<<23) /* 8M bytes = 32Mbits */
uint8_t _nvf_data[NVF_NUM_DEVICES][NVF_CHIP_SIZE/NVF_PAGE_SIZE][NVF_PAGE_SIZE];
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
  uint8_t _selected = addr >> (16 - NVF_MAX_DEVICES_LOGN2);
  addr <<= NVF_MAX_DEVICES_LOGN2; addr >>= NVF_MAX_DEVICES_LOGN2;
  addr <<= 1;

  for (uint16_t ui16_1=0; ui16_1<16; ui16_1++, addr++) {
    memset(((uint8_t *)_nvf_data)+(NVF_CHIP_SIZE*_selected)+(addr*NVF_PAGE_SIZE), 0xFF, NVF_PAGE_SIZE);
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

uint16_t
bill_write_bytes(uint16_t addr, uint8_t* buf, uint16_t len)
{
  assert(0 == _nvf_sleep);

  uint8_t  ui8_1;
  uint16_t ui16_1;
  uint8_t* lbuf = buf;

  uint8_t _selected = addr >> (16 - NVF_MAX_DEVICES_LOGN2);
  addr <<= NVF_MAX_DEVICES_LOGN2; addr >>= NVF_MAX_DEVICES_LOGN2;
  addr <<= 1;

  for (ui8_1=0, ui16_1=0; len; len--, ui8_1++, ui16_1++) {
    _nvf_data[_selected][addr][ui8_1] = (NULL == buf) ? 0 : *lbuf++;
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
  uint8_t _selected = addr >> (16 - NVF_MAX_DEVICES_LOGN2);
  addr <<= NVF_MAX_DEVICES_LOGN2; addr >>= NVF_MAX_DEVICES_LOGN2;
  addr <<= 1;

  for (ui8_1=0, ui16_1=0; len; len--, ui8_1++, ui16_1++, buf++) {
    buf[0] = _nvf_data[_selected][addr][ui8_1];
    addr += (0xFF == ui8_1) ? 1 : 0;
  }

  return ui16_1;
}

/* Item address always falls in 0th device.
   addr[15:0] is 4 byte aligned corresponds to [17:2] of linear addr map
   max addr space is 256KBytes
 */
uint16_t
item_read_bytes(uint16_t addr, uint8_t* buf, uint16_t len)
{
  assert(len > 0);
  assert(len <= NVF_PAGE_SIZE);
  if (len == NVF_PAGE_SIZE)
    assert(0 == ((addr<<ITEM_ADDR_SHIFT)&0xFF));
  assert(0 == _nvf_sleep);
  assert(buf);

  /* 256 byte page */
  assert(4 == ITEM_ADDR_SHIFT); /* else the below logic fails */
  uint8_t  ui8_1 = (addr&0xF)<<ITEM_ADDR_SHIFT;
  uint16_t ui16_1;
  uint8_t _selected = 0;
  printf("item_read_bytes: addr:0x%x buf:%p len:%d ui8_1:0x%x\n", addr, buf, len, ui8_1);
  addr >>= (8-ITEM_ADDR_SHIFT);

  for (ui16_1=0; len; len--, ui8_1++, ui16_1++, buf++) {
    buf[0] = _nvf_data[_selected][addr][ui8_1];
    addr += (0xFF == ui8_1) ? 1 : 0;
  }

  return ui16_1;
}

uint16_t
item_write_bytes(uint16_t addr, uint8_t* buf, uint16_t len)
{
  assert(len > 0);
  assert(len <= NVF_PAGE_SIZE);
  if (len == NVF_PAGE_SIZE)
    assert(0 == ((addr<<ITEM_ADDR_SHIFT)&0xFF));
  assert(0 == _nvf_sleep);

  assert(4 == ITEM_ADDR_SHIFT); /* else the below logic fails */
  uint8_t  ui8_1 = (addr&0xF)<<ITEM_ADDR_SHIFT;
  uint16_t ui16_1;
  uint8_t* lbuf = buf;
  uint8_t _selected = 0;
  printf("item_write_bytes: addr:0x%x buf:%p len:%d ui8_1:0x%x\n", addr, buf, len, ui8_1);
  addr >>= (8-ITEM_ADDR_SHIFT);

  for (ui16_1=0; len; len--, ui8_1++, ui16_1++) {
    _nvf_data[_selected][addr][ui8_1] = (NULL == buf) ? 0 : *lbuf++;
    addr += (0xFF == ui8_1) ? 1 : 0;
  }

  return ui16_1;
}
