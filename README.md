# ATMEGA Billing Machine
Documentation at [website](https://narenkn.com/works/embedded/).

## Installation
1. sudo apt-get install gcc-avr binutils-avr avr-libc
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
