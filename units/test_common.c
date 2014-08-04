#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include <assert.c>

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/crc16.h>

#define TEST_KEY_ARR_SIZE 128

#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#include "billing.h"
#include "i2c.h"
#include "uart.h"
#include "a1micro2mm.h"
#include "menu.h"
#include "main.h"

#include "lcd.c"
#include "kbd.c"
#include "ep_store.c"
#include "i2c.c"
#include "uart.c"
#include "ff.c"
#include "a1micro2mm.c"
#include "menu.c"

uint8_t inp[TEST_KEY_ARR_SIZE], inp2[TEST_KEY_ARR_SIZE];
uint8_t inp3[TEST_KEY_ARR_SIZE], inp4[TEST_KEY_ARR_SIZE];

