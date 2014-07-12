#ifndef AVR_PGMSPACE_H
#define AVR_PGMSPACE_H

#define PROGMEM
#define EEMEM

#define pgm_read_mem(A)  (A)[0]

#endif

