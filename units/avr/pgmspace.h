#ifndef AVR_PGMSPACE_H
#define AVR_PGMSPACE_H

#define PROGMEM
#define EEMEM

#define PSTR(A)          ((const char *)A)

#define pgm_read_mem(addr)       ((uint8_t *)addr)[0]

#endif
