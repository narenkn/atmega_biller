# ATMEGA128 Billing Machine
Documentation at [website](https://narenkn.com/works/embedded/at128_biller/).

This branch _atmega128_no_eeprom_ should be kept in sync with _atmega128_ branch. The only difference should be both items and bills are stored in Non-Volatile Flash in this branch, whereas items is stored in I2C EEPROMs in the other.

## Installation
1. sudo apt-get install gcc-avr
1. sudo apt install libncurses5-dev

## Run Unit tests
1. cd units
1. make SOURCE=<test_name.c>

## Build/ Running Board Tests
1. cd tests
1. make SOURCE=<test_name.c>

## Build the final image
1. cd default
1. make
