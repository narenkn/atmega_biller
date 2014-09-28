#ifndef AVR_EEPROM_H
#define AVR_EEPROM_H

#define EEMEM
#define AVR_EEPROM_SIZE (1<<10)

uint8_t _avr_eeprom[AVR_EEPROM_SIZE];

#define EEPROM_DIRECT_ASSIGN(addr, val) \
  _avr_eeprom[addr] = val;

uint8_t
eeprom_read_byte (const uint8_t *addr)
{
  uint16_t a = (uint16_t) addr;
  assert(a < AVR_EEPROM_SIZE);
  if (a >= AVR_EEPROM_SIZE)
    printf("a:0x%x\n", a);
  return _avr_eeprom[a];
}

void
eeprom_update_byte (uint8_t *addr, uint8_t value)
{
  uint16_t a = (uint16_t) addr;
  assert(a < AVR_EEPROM_SIZE);
  _avr_eeprom[a] = value;
}

uint16_t
eeprom_read_word (const uint16_t *addr)
{
  uint16_t a = (uint16_t) addr;
  assert(a < AVR_EEPROM_SIZE);
  return (_avr_eeprom[a]<<8)|_avr_eeprom[a+1];
}

void
eeprom_update_word (uint16_t *addr, uint16_t value)
{
  uint16_t a = (uint16_t) addr;
  assert(a < AVR_EEPROM_SIZE);
  _avr_eeprom[a+1] = value;
  value >>= 8;
  _avr_eeprom[a] = value;
}

uint16_t
eeprom_read_dword (const uint32_t *addr)
{
  assert(0);
}

void
eeprom_update_dword (uint32_t *addr, uint32_t value)
{
  assert(0);
}

uint16_t
eeprom_read_float (const float *addr)
{
  assert(0);
}

void
eeprom_update_float (float *addr, float value)
{
  assert(0);
}

void
eeprom_read_block (void *pointer_ram, const void *pointer_eeprom, size_t n)
{
  size_t ui1, ui2 = (size_t)pointer_eeprom;
  for (ui1=0; ui1<n; ui1++, ui2++) {
    /* FIXME: don't allow page crossover */
    ((uint8_t *)pointer_ram)[ui1] = _avr_eeprom[ui2];
  }
}

void
eeprom_update_block (const void *pointer_ram, void *pointer_eeprom, size_t n)
{
  size_t ui1, ui2 = (size_t)pointer_eeprom;
  for (ui1=0; ui1<n; ui1++, ui2++) {
    /* FIXME: don't allow page crossover */
    _avr_eeprom[ui2] = ((uint8_t *)pointer_ram)[ui1];
  }
}

#define eeprom_busy_wait()

#endif
