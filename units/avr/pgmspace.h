#ifndef AVR_PGMSPACE_H
#define AVR_PGMSPACE_H

#define PROGMEM
#define EEMEM

#define PSTR(A)          ((const char *)A)

#define pgm_read_byte pgm_read_byte_near
#define pgm_read_byte_near(addr)       ((uint8_t *)addr)[0]
#define strncpy_P strncpy
#define strcpy_P strcpy

#define pgm_read_word(addr) (		    \
  (((uint16_t)(((uint8_t *)addr)[1]))<<8) | \
  (((uint8_t *)addr)[0])		    \
					    )

#define pgm_read_dword(addr) (		    \
  (((uint32_t)(((uint8_t *)addr)[3]))<<24)| \
  (((uint32_t)(((uint8_t *)addr)[2]))<<16)| \
  (((uint32_t)(((uint8_t *)addr)[1]))<<8) | \
  (((uint8_t *)addr)[0])		    \
					    )

#endif
