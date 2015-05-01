#ifndef DEVICE_H
#define DEVICE_H

#if defined (__AVR_ATmega32__)
# define FLASH_PAGE_SIZE_LOGN     7
#elif defined (__AVR_ATxmega64A1__) || defined (__AVR_ATxmega64A3__) || defined (__AVR_ATxmega64D3__)
# define FLASH_PAGE_SIZE_LOGN     7
#elif defined (__AVR_ATxmega128A1__) || defined (__AVR_ATxmega128A3__) || defined (__AVR_ATxmega128D3__) || defined (__AVR_ATmega1284P__)
# define FLASH_PAGE_SIZE_LOGN     8
#elif defined (__AVR_ATxmega256A3__) || defined (__AVR_ATxmega256A3B__) || defined (__AVR_ATxmega256D3__)
# define FLASH_PAGE_SIZE_LOGN     8
#else
# error "Not a known device"
#endif

#endif
