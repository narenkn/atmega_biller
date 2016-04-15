#if defined (__AVR_ATmega32__)
#include "uart_at32.c"
#else
# error "Unknown processor"
#endif
