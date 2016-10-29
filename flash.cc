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
#include "menu.h"
#include "main.h"

//uint8_t SPIFlash::UNIQUEID[8];

/// IMPORTANT: NAND FLASH memory requires erase before write, because
///            it can only transition from 1s to 0s and only the erase command can reset all 0s to 1s
/// See http://en.wikipedia.org/wiki/Flash_memory
/// The smallest range that can be erased is a sector (4K, 32K, 64K); there is also a chip erase command

/// Constructor. JedecID is optional but recommended, since this will ensure that the device is present and has a valid response
/// get this from the datasheet of your flash chip
/// Example for Atmel-Adesto 4Mbit AT25DF041A: 0x1F44 (page 27: http://www.adestotech.com/sites/default/files/datasheets/doc3668.pdf)
/// Example for Winbond 4Mbit W25X40CL: 0xEF30 (page 14: http://www.winbond.com/NR/rdonlyres/6E25084C-0BFE-4B25-903D-AE10221A0929/0/W25X40CL.pdf)

/// Select the flash chip
void
SPIFlash::select()
{
  PORTF &= ~(_BV(_selected)&0xF);
}

/// UNselect the flash chip
void
SPIFlash::unselect()
{
  PORTF |= 0xF;
}

/// setup SPI, read device ID etc...
bool
SPIFlash::initialize()
{
  unselect();
  wakeup();

  /* all flash devices */
  for (uint8_t id=0; id<EEPROM_MAX_DEVICES; id++) {
    if (JEDEC_ID != readDeviceId())
      return false;

    command(SPIFLASH_STATUSWRITE, true); // Write Status Register
    spiTransmit(0);                     // Global Unprotect
    unselect();
  }

  return true;
}

/// Get the manufacturer and device ID bytes (as a short word)
uint32_t
SPIFlash::readDeviceId()
{
  select();
  spiTransmit(SPIFLASH_IDREAD);
  uint32_t jedecid = spiTransmit(0);
  jedecid <<= 8;
  jedecid |= spiTransmit(0);
  jedecid <<= 8;
  jedecid |= spiTransmit(0);
  unselect();
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
//  command(SPIFLASH_MACREAD);
//  spiTransmit(0);
//  spiTransmit(0);
//  spiTransmit(0);
//  spiTransmit(0);
//  for (uint8_t i=0;i<8;i++)
//    UNIQUEID[i] = spiTransmit(0);
//  unselect();
//  return UNIQUEID;
//}

/// read unlimited # of bytes
uint16_t
SPIFlash::readBytes(uint16_t addr, uint8_t* buf, uint16_t len)
{
  _selected = addr >> (16 - EEPROM_MAX_DEVICES_LOGN2);
  addr <<= EEPROM_MAX_DEVICES_LOGN2;

  command(SPIFLASH_ARRAYREAD);
  /* coded for 8Mbyte device where address[23] is don't care */
  spiTransmit(addr >> 8);
  spiTransmit(addr);
  spiTransmit(0); /* addr[7:0] */
  spiTransmit(0); //"dont care"
  for (uint16_t i = 0; i < len; ++i)
    ((uint8_t*) buf)[i] = spiTransmit(0);
  unselect();

  return len;
}

/// Send a command to the flash chip, pass TRUE for isWrite when its a write command
void
SPIFlash::command(uint8_t cmd, bool isWrite)
{
  if (isWrite)
  {
    command(SPIFLASH_WRITEENABLE); // Write Enable
    unselect();
  }

  //wait for any write/erase to complete
  //  a time limit cannot really be added here without it being a very large safe limit
  //  that is because some chips can take several seconds to carry out a chip erase or other similar multi block or entire-chip operations
  //  a recommended alternative to such situations where chip can be or not be present is to add a 10k or similar weak pulldown on the
  //  open drain MISO input which can read noise/static and hence return a non 0 status byte, causing the while() to hang when a flash chip is not present
  while(busy());
  select();
  spiTransmit(cmd);
}

/// check if the chip is busy erasing/writing
bool
SPIFlash::busy()
{
  /*
  select();
  spiTransmit(SPIFLASH_STATUSREAD);
  uint8_t status = spiTransmit(0);
  unselect();
  return status & 1;
  */
  return readStatus() & 1;
}

/// return the STATUS register
uint8_t
SPIFlash::readStatus()
{
  select();
  spiTransmit(SPIFLASH_STATUSREAD);
  uint8_t status = spiTransmit(0);
  unselect();
  return status;
}


/// write multiple bytes to flash memory (up to 64K)
/// WARNING: you can only write to previously erased memory locations (see datasheet)
///          use the block erase commands to first clear memory (write 0xFFs)
/// This version handles both page alignment and data blocks larger than 256 bytes.
///
uint16_t
SPIFlash::writeBytes(uint16_t addr, uint8_t* buf, uint16_t len)
{
  uint16_t _len = len;
  // force the first set of bytes to stay within the first page
  uint16_t maxBytesInPage;

  _selected = addr >> (16 - EEPROM_MAX_DEVICES_LOGN2);
  addr <<= EEPROM_MAX_DEVICES_LOGN2;

  for (uint16_t idx=0, offset=0; _len>0; idx++)
  {
    /* coded for 8Mbyte device where address[23] is don't care */
    command(SPIFLASH_BYTEPAGEPROGRAM, true);  // Byte/Page Program
    spiTransmit((addr+idx) >> 8);
    spiTransmit(addr+idx);
    spiTransmit(0); /* addr[7:0] */

    maxBytesInPage = (_len<=NVF_PAGE_SIZE) ? _len : NVF_PAGE_SIZE;
    for (uint16_t i = 0; i < maxBytesInPage; i++)
      spiTransmit(((uint8_t*) buf)[offset + i]);
    unselect();

    offset += maxBytesInPage;
    _len -= maxBytesInPage;
    maxBytesInPage = NVF_PAGE_SIZE;   // now we can do up to full page
  }

  return len;
}

/// erase entire flash memory array
/// may take several seconds depending on size, but is non blocking
/// so you may wait for this to complete using busy() or continue doing
/// other things and later check if the chip is done with busy()
/// note that any command will first wait for chip to become available using busy()
/// so no need to do that twice
void
SPIFlash::chipErase(uint8_t sel)
{
  _selected = sel;
  command(SPIFLASH_CHIPERASE, true);
  unselect();
}

/// erase a 4Kbyte block
void
SPIFlash::blockErase4K(uint16_t addr)
{
  _selected = addr >> (16 - EEPROM_MAX_DEVICES_LOGN2);
  addr <<= EEPROM_MAX_DEVICES_LOGN2;

  command(SPIFLASH_BLOCKERASE_4K, true); // Block Erase
  spiTransmit(addr >> 8);
  spiTransmit(addr);
  spiTransmit(0);
  unselect();
}

void
SPIFlash::sleep()
{
  command(SPIFLASH_SLEEP);
  unselect();
}

void
SPIFlash::wakeup()
{
  command(SPIFLASH_WAKE);
  unselect();
}

SPIFlash spiFlash;
