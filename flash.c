// Copyright (c) 2013-2015 by Felix Rusu, LowPowerLab.com
// SPI Flash memory library for arduino/moteino.
// This works with 256byte/page SPI flash memory
// For instance a 4MBit (512Kbyte) flash chip will have 2048 pages: 256*2048 = 524288 bytes (512Kbytes)
// Minimal modifications should allow chips that have different page size but modifications
// DEPENDS ON: Arduino SPI library
// > Updated Jan. 5, 2015, TomWS1, modified writeBytes to allow blocks > 256 bytes and handle page misalignment.
// > Updated Feb. 26, 2015 TomWS1, added support for SPI Transactions (Arduino 1.5.8 and above)
// > Selective merge by Felix after testing in IDE 1.0.6, 1.6.4
// **********************************************************************************
// License
// **********************************************************************************
// This program is free software; you can redistribute it 
// and/or modify it under the terms of the GNU General    
// Public License as published by the Free Software       
// Foundation; either version 3 of the License, or        
// (at your option) any later version.                    
//                                                        
// This program is distributed in the hope that it will   
// be useful, but WITHOUT ANY WARRANTY; without even the  
// implied warranty of MERCHANTABILITY or FITNESS FOR A   
// PARTICULAR PURPOSE. See the GNU General Public        
// License for more details.                              
//                                                        
// You should have received a copy of the GNU General    
// Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.
//                                                        
// Licence can be viewed at                               
// http://www.gnu.org/licenses/gpl-3.0.txt
//
// Please maintain this license information along with authorship
// and copyright notices in any redistribution of this code

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <avr/eeprom.h>
#include <util/twi.h>

#include "ep_ds.h"
#include "version.h"
#include "assert.h"
#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#include "billing.h"
#include "i2c.h"
#include "flash.h"
#include "uart.h"
#include "spi.h"
#include "a1micro2mm.h"
#include "main.h"
#include "menu.h"

//uint8_t SPIFlash::UNIQUEID[8];

/// IMPORTANT: NAND FLASH memory requires erase before write, because
///            it can only transition from 1s to 0s and only the erase command can reset all 0s to 1s
/// See http://en.wikipedia.org/wiki/Flash_memory
/// The smallest range that can be erased is a sector (4K, 32K, 64K); there is also a chip erase command

/// Constructor. JedecID is optional but recommended, since this will ensure that the device is present and has a valid response
/// get this from the datasheet of your flash chip
/// Example for Atmel-Adesto 4Mbit AT25DF041A: 0x1F44 (page 27: http://www.adestotech.com/sites/default/files/datasheets/doc3668.pdf)
/// Example for Winbond 4Mbit W25X40CL: 0xEF30 (page 14: http://www.winbond.com/NR/rdonlyres/6E25084C-0BFE-4B25-903D-AE10221A0929/0/W25X40CL.pdf)

static uint8_t _selected = 0;

static void nvfCommand(uint8_t cmd, bool isWrite);
static uint8_t nvfReadStatus();
static bool nvfBusy();
void nvfChipErase(uint8_t sel);
void nvfBlockErase4K(uint16_t address);
static uint32_t nvfReadDeviceId();
  
void nvfSleep();
void nvfWakeUp();
static void nvfSelect();
static void nvfUnSelect();

/// Select the flash chip
static void
nvfSelect()
{
  PORTF &= ~(_BV(_selected)&0xF);
}

/// NvfUnSelect the flash chip
static void
nvfUnSelect()
{
  PORTF |= 0xF;
}

/// setup SPI, read device ID etc...
bool
nvfInit()
{
  bool ret=false;

  /* all flash devices */
  for (uint8_t id=0; id<NVF_NUM_DEVICES; id++) {
    _selected = id;
    nvfUnSelect();
    nvfWakeUp();
  }
  _delay_ms(500);
  ret = true;

  for (uint8_t id=0; id<NVF_NUM_DEVICES; id++) {
    _selected = id;
    uint32_t ui1 = nvfReadDeviceId();
    ret &= (JEDEC_ID == ui1);

    nvfCommand(SPIFLASH_STATUSWRITE, true); // Write Status Register
    spiTransmit(0);                     // Global Unprotect
    nvfUnSelect();
  }
  _delay_ms(500);

  /* NVF_SALE_START_ADDRESS should be 4k aligned : min erase is 4k size.
   *   It contains 22:9 of 8M addr space */
  assert(0 == (NVF_SALE_START_ADDR&0x7));

  return ret;
}

/// Get the manufacturer and device ID bytes (as a short word)
static uint32_t
nvfReadDeviceId()
{
  nvfSelect();
  spiTransmit(SPIFLASH_IDREAD);
  uint32_t jedecid = spiTransmit(0);
  jedecid <<= 8;
  jedecid |= spiTransmit(0);
  jedecid <<= 8;
  jedecid |= spiTransmit(0);
  nvfUnSelect();
  return jedecid;
}

///// Get the 64 bit unique identifier, stores it in UNIQUEID[8]. Only needs to be called once, ie after initialize
///// Returns the byte pointer to the UNIQUEID byte array
///// Read UNIQUEID like this:
///// flash.readUniqueId(); for (uint8_t i=0;i<8;i++) { Serial.print(flash.UNIQUEID[i], HEX); Serial.print(' '); }
///// or like this:
///// flash.readUniqueId(); uint8_t* MAC = flash.readUniqueId(); for (uint8_t i=0;i<8;i++) { Serial.print(MAC[i], HEX); Serial.print(' '); }
//uint8_t*
//SPIFlash::readUniqueId()
//{
//  nvfCommand(SPIFLASH_MACREAD, false);
//  spiTransmit(0);
//  spiTransmit(0);
//  spiTransmit(0);
//  spiTransmit(0);
//  for (uint8_t i=0;i<8;i++)
//    UNIQUEID[i] = spiTransmit(0);
//  nvfUnSelect();
//  return UNIQUEID;
//}

/// read unlimited # of bytes
uint16_t
bill_read_bytes(uint16_t addr, uint8_t* buf, uint16_t len)
{
  _selected = addr >> (16 - NVF_MAX_DEVICES_LOGN2);
  addr <<= NVF_MAX_DEVICES_LOGN2; addr >>= NVF_MAX_DEVICES_LOGN2;
  addr <<= 1;

  nvfCommand(SPIFLASH_ARRAYREAD, false);
  /* coded for 8Mbyte device, address[23] is don't care */
  spiTransmit(addr >> 8);
  spiTransmit(addr);
  spiTransmit(0); /* addr[7:0] */
  spiTransmit(0); //"dont care"
  uint16_t _len;
  for (_len = 0; _len < len; ++_len) {
    buf[0] = spiTransmit(0);
    buf++;
  }
  nvfUnSelect();

  return _len;
}

/* addr[15:0] is 16-byte aligned corresponds to flash-addr[19:4]
   addr doesn't contain flash select bits, flash-0 is always selected.
 */
uint16_t
item_read_bytes(uint16_t addr, uint8_t* buf, uint16_t len)
{
  assert(len > 0);
  assert(len <= NVF_PAGE_SIZE);
  if (len == NVF_PAGE_SIZE)
    assert(0 == ((addr<<ITEM_ADDR_SHIFT)&0xFF));
  _selected = 0;

  nvfCommand(SPIFLASH_ARRAYREAD, false);
  assert(4 == ITEM_ADDR_SHIFT); /* else the below logic doesn't work */
  /* coded for 8Mbyte device, address[23] is don't care */
  spiTransmit(addr>>12);  /* addr[23:16] */
  spiTransmit(addr>>4);   /* addr[15:8] */
  spiTransmit(addr<<4);   /* addr[7:0] */
  spiTransmit(0); //"dont care"
  for (uint16_t i = 0; i < len; ++i)
    ((uint8_t*) buf)[i] = spiTransmit(0);
  nvfUnSelect();

  return len;
}

/// Send a command to the flash chip, pass TRUE for isWrite when its a write command
static void
nvfCommand(uint8_t cmd, bool isWrite)
{
  if (isWrite)
  {
    nvfCommand(SPIFLASH_WRITEENABLE, false); // Write Enable
    nvfUnSelect();
  }

  //wait for any write/erase to complete
  //  a time limit cannot really be added here without it being a very large safe limit
  //  that is because some chips can take several seconds to carry out a chip erase or other similar multi block or entire-chip operations
  //  a recommended alternative to such situations where chip can be or not be present is to add a 10k or similar weak pulldown on the
  //  open drain MISO input which can read noise/static and hence return a non 0 status byte, causing the while() to hang when a flash chip is not present
  while(nvfBusy());
  nvfSelect();
  spiTransmit(cmd);
}

/// check if the chip is busy erasing/writing
static bool
nvfBusy()
{
  /*
  nvfSelect();
  spiTransmit(SPIFLASH_STATUSREAD);
  uint8_t status = spiTransmit(0);
  nvfUnSelect();
  return status & 1;
  */
  return nvfReadStatus() & 1;
}

/// return the STATUS register
static uint8_t
nvfReadStatus()
{
  nvfSelect();
  spiTransmit(SPIFLASH_STATUSREAD);
  uint8_t status = spiTransmit(0);
  nvfUnSelect();
  return status;
}


/// write multiple bytes to flash memory (up to 64K)
/// WARNING: you can only write to previously erased memory locations (see datasheet)
///          use the block erase commands to first clear memory (write 0xFFs)
/// This version handles both page alignment and data blocks larger than 256 bytes.
///
uint16_t
bill_write_bytes(uint16_t addr, uint8_t* buf, uint16_t len)
{
  uint16_t _len = len;
  // force the first set of bytes to stay within the first page
  uint16_t maxBytesInPage;

  _selected = addr >> (16 - NVF_MAX_DEVICES_LOGN2);
  addr <<= NVF_MAX_DEVICES_LOGN2; addr >>= NVF_MAX_DEVICES_LOGN2;
  addr <<= 1;

  for (uint16_t idx=0, offset=0; _len>0; idx++)
  {
    /* coded for 8Mbyte device where address[23] is don't care */
    nvfCommand(SPIFLASH_BYTEPAGEPROGRAM, true);  // Byte/Page Program
    spiTransmit((addr+idx) >> 8);
    spiTransmit(addr+idx);
    spiTransmit(0); /* addr[7:0] */

    maxBytesInPage = (_len<=NVF_PAGE_SIZE) ? _len : NVF_PAGE_SIZE;
    for (uint16_t i=maxBytesInPage; i; --i, --_len) {
      spiTransmit(buf[offset++]);
    }
    nvfUnSelect();
  }

  return len-_len;
}

/* FIXME: Not completed
 * Bytes would never be crossing page boundry.
 * if (NULL == buf) program 0's;
 *   To mark a item as "deleted" we program 0 to CRC, nCRC
 * Assumption is that page/sector is already erased and no
 *   overlap writes happen.
 */
uint16_t
item_write_bytes(uint16_t addr, uint8_t* buf, uint16_t len)
{
  assert(len > 0);
  assert(len <= NVF_PAGE_SIZE);
  if (len == NVF_PAGE_SIZE)
    assert(0 == ((addr<<ITEM_ADDR_SHIFT)&0xFF));
  _selected = 0;

  /* coded for 8Mbyte device where address[23] is don't care */
  assert(4 == ITEM_ADDR_SHIFT); /* else the below logic doesn't work */
  nvfCommand(SPIFLASH_BYTEPAGEPROGRAM, true);  // Byte/Page Program
  spiTransmit(addr>>12);  /* addr[23:16] */
  spiTransmit(addr>>4);   /* addr[15:8] */
  spiTransmit(addr<<4);   /* addr[7:0] */

  for (uint16_t i = 0; i < len; i++)
    spiTransmit(buf[len]);
  nvfUnSelect();

  return len;
}

/// erase entire flash memory array
/// may take several seconds depending on size, but is non blocking
/// so you may wait for this to complete using busy() or continue doing
/// other things and later check if the chip is done with busy()
/// note that any command will first wait for chip to become available using busy()
/// so no need to do that twice
void
nvfChipErase(uint8_t sel)
{
  _selected = sel;
  nvfCommand(SPIFLASH_CHIPERASE, true);
  nvfUnSelect();

  while(nvfBusy());
}

/// erase a 4Kbyte block
void
nvfBlockErase4K(uint16_t addr)
{
  _selected = addr >> (16 - NVF_MAX_DEVICES_LOGN2);
  addr <<= NVF_MAX_DEVICES_LOGN2; addr >>= NVF_MAX_DEVICES_LOGN2;
  addr <<= 1;

  nvfCommand(SPIFLASH_BLOCKERASE_4K, true); // Block Erase
  spiTransmit(addr >> 8);
  spiTransmit(addr);
  spiTransmit(0);
  nvfUnSelect();

  while(nvfBusy());
}

void
nvfSleep()
{
  nvfCommand(SPIFLASH_SLEEP, false);
  nvfUnSelect();
}

void
nvfWakeUp()
{
  nvfCommand(SPIFLASH_WAKE, false);
  spiTransmit(0);
  spiTransmit(0);
  spiTransmit(0);
  spiTransmit(0);
  nvfUnSelect();
}

