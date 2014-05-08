#ifndef MAIN_H
#define MAIN_H

void main_init(void);

#define BUZZER_ON  PORTD |= 0x80
#define BUZZER_OFF PORTD &= ~0x80

#endif
