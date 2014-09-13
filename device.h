#ifndef DEVICE_H
#define DEVICE_H

#if      ATMega32
# define FLASH_PAGE_SIZE_LOGN     7
#elif    ATxMega64
# define FLASH_PAGE_SIZE_LOGN     7
#elif  ATxMega128
# define FLASH_PAGE_SIZE_LOGN     8
#elif  ATxMega256
# define FLASH_PAGE_SIZE_LOGN     8
#else
# error "Not a known device"
#endif

#endif
