#if defined (__AVR_ATmega32__)
#include "uart_at32.c"
#elif defined (__AVR_ATmega1284P__)
#include "uart_at1284.c"
#else
# error "Unknown processor"
#endif
