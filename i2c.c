//************************************************************
//    ******** FUNCTIONS FOR I2C COMMUNICATION  *******
//************************************************************
//Controller		: ATmega32 (Clock: 8 Mhz-internal)
//Compiler			: AVR-GCC (winAVR with AVRStudio-4)
//Project Version	: DL_1.0
//Author			: CC Dharmani, Chennai (India)
//			  		  www.dharmanitech.com
//Date				: 10 May 2011
//************************************************************

#include "i2c.h"

//************************************************
// TWI initialize
// bit rate:18 (freq: 100Khz @16MHz)
//************************************************
void
i2c_init(void)
{
#if 1
  TWCR= 0x00; //disable twi
  TWBR= 0x12; //set bit rate
  TWSR= 0x00; //set prescale
  //TWCR= 0x44; //enable twi
#endif
#if 0
  /* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */
#if defined(TWPS0)
  /* has prescaler (mega128 & newer) */
  TWSR = 0;
#endif

#if F_CPU < 3600000UL
  TWBR = 10;			/* smallest TWBR value, see note [5] */
#else
  TWBR = (F_CPU / 100000UL - 16) / 2;
#endif
#endif
}


//*************************************************
//Function to start i2c communication
//*************************************************
uint8_t
i2c_start(void)
{
  TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); 	//Send START condition
	
  while (!(TWCR & (1<<TWINT)));   		//Wait for TWINT flag set. This indicates that the
  //START condition has been transmitted
  if ((TWSR & 0xF8) == START)			//Check value of TWI Status Register
    return(0);
  else
    return(1);
}

//*************************************************
//Function for repeat start condition
//*************************************************
uint8_t
i2c_repeatStart(void)
{
  TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); 		//Send START condition
  while (!(TWCR & (1<<TWINT)));   		//Wait for TWINT flag set. This indicates that the
  //START condition has been transmitted
  if ((TWSR & 0xF8) == REPEAT_START)			//Check value of TWI Status Register
    return(0);
  else
    return(1);
}

//**************************************************
//Function to transmit address of the slave
//*************************************************
uint8_t
i2c_sendAddress(uint8_t address)
{
  uint8_t STATUS;

  if((address & 0x01) == 0) 
    STATUS = MT_SLA_ACK;
  else
    STATUS = MR_SLA_ACK; 

  TWDR = address; 
  TWCR = (1<<TWINT)|(1<<TWEN);	   //Load SLA_W into TWDR Register. Clear TWINT bit
  //in TWCR to start transmission of address
  while (!(TWCR & (1<<TWINT)));	   //Wait for TWINT flag set. This indicates that the
  //SLA+W has been transmitted, and
  //ACK/NACK has been received.
  if ((TWSR & 0xF8) == STATUS)	   //Check value of TWI Status Register
    return(0);
  else 
    return(1);
}

//**************************************************
//Function to transmit a data byte
//*************************************************
uint8_t
i2c_sendData(uint8_t data)
{
  TWDR = data; 
  TWCR = (1<<TWINT) |(1<<TWEN);	   //Load SLA_W into TWDR Register. Clear TWINT bit
  //in TWCR to start transmission of data
  while (!(TWCR & (1<<TWINT)));	   //Wait for TWINT flag set. This indicates that the
  //data has been transmitted, and
  //ACK/NACK has been received.
  if ((TWSR & 0xF8) != MT_DATA_ACK)   //Check value of TWI Status Register
    return(1);
  else
    return(0);
}

//*****************************************************
//Function to receive a data byte and send ACKnowledge
//*****************************************************
uint8_t
i2c_receiveData_ACK(void)
{
  uint8_t data;
  
  TWCR = (1<<TWEA)|(1<<TWINT)|(1<<TWEN);
  
  while (!(TWCR & (1<<TWINT)));	   	   //Wait for TWINT flag set. This indicates that the
  //data has been received
  if ((TWSR & 0xF8) != MR_DATA_ACK)    //Check value of TWI Status Register
    return(ERROR_CODE);
  
  data = TWDR;
  return(data);
}

//******************************************************************
//Function to receive the last data byte (no acknowledge from master
//******************************************************************
uint8_t
i2c_receiveData_NACK(void)
{
  uint8_t data;
  
  TWCR = (1<<TWINT)|(1<<TWEN);
  
  while (!(TWCR & (1<<TWINT)));	   	   //Wait for TWINT flag set. This indicates that the
  //data has been received
  if ((TWSR & 0xF8) != MR_DATA_NACK)    //Check value of TWI Status Register
    return(ERROR_CODE);
  
  data = TWDR;
  return(data);
}

//**************************************************
//Function to end the i2c communication
//*************************************************   	
void
i2c_stop(void)
{
  TWCR =  (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);	  //Transmit STOP condition
}  

/*
 * Note [7]
 *
 * Read "len" bytes from EEPROM starting at "eeaddr" into "buf".
 *
 * This requires two bus cycles: during the first cycle, the device
 * will be selected (master transmitter mode), and the address
 * transfered.
 * Address bits exceeding 256 are transfered in the
 * E2/E1/E0 bits (subaddress bits) of the device selector.
 * Address is sent in two dedicated 8 bit transfers
 * for 16 bit address devices (larger EEPROM devices)
 *
 * The second bus cycle will reselect the device (repeated start
 * condition, going into master receiver mode), and transfer the data
 * from the device to the TWI master.  Multiple bytes can be
 * transfered by ACKing the client's transfer.  The last transfer will
 * be NACKed, which the client will take as an indication to not
 * initiate further transfers.
 */
uint16_t
ee24xx_read_bytes(uint16_t eeaddr, uint8_t *buf, uint16_t len)
{
  uint8_t sla, twcr, n = 0;
  uint16_t rv = 0;

#ifndef WORD_ADDRESS_16BIT
  /* patch high bits of EEPROM address into SLA */
  sla = TWI_SLA_24CXX | (((eeaddr >> 8) & 0x07) << 1);
#else
  /* 16-bit address devices need only TWI Device Address */
  sla = TWI_SLA_24CXX;
#endif

  /*
   * Note [8]
   * First cycle: master transmitter mode
   */
 restart:
  if (n++ >= MAX_ITER)
    return -1;
 begin:

  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send start condition */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS)) {
  case TW_REP_START:		/* OK, but should not happen */
  case TW_START:
    break;

  case TW_MT_ARB_LOST:	/* Note [9] */
    goto begin;

  default:
    return -1;		/* error: not in start condition */
    /* NB: do /not/ send stop condition */
  }

  /* Note [10] */
  /* send SLA+W */
  TWDR = sla | TW_WRITE;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS)) {
  case TW_MT_SLA_ACK:
    break;

  case TW_MT_SLA_NACK:	/* nack during select: device busy writing */
    /* Note [11] */
    goto restart;

  case TW_MT_ARB_LOST:	/* re-arbitrate */
    goto begin;

  default:
    goto error;		/* must send stop condition */
  }

#ifdef WORD_ADDRESS_16BIT
  TWDR = (eeaddr >> 8);		/* 16-bit word address device, send high 8 bits of addr */
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS)) {
  case TW_MT_DATA_ACK:
    break;

  case TW_MT_DATA_NACK:
    goto quit;

  case TW_MT_ARB_LOST:
    goto begin;

  default:
    goto error;		/* must send stop condition */
  }
#endif

  TWDR = eeaddr;		/* low 8 bits of addr */
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0); /* wait for transmission */
  switch ((twst = TW_STATUS)) {
  case TW_MT_DATA_ACK:
    break;

  case TW_MT_DATA_NACK:
    goto quit;

  case TW_MT_ARB_LOST:
    goto begin;

  default:
    goto error;		/* must send stop condition */
  }

  /*
   * Note [12]
   * Next cycle(s): master receiver mode
   */
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send (rep.) start condition */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS)) {
  case TW_START:		/* OK, but should not happen */
  case TW_REP_START:
    break;

  case TW_MT_ARB_LOST:
    goto begin;

  default:
    goto error;
  }

  /* send SLA+R */
  TWDR = sla | TW_READ;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS)) {
  case TW_MR_SLA_ACK:
    break;

  case TW_MR_SLA_NACK:
    goto quit;

  case TW_MR_ARB_LOST:
    goto begin;

  default:
    goto error;
  }

  for (twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWEA) /* Note [13] */;
       len > 0;
       len--) {
    if (len == 1)
      twcr = _BV(TWINT) | _BV(TWEN); /* send NAK this time */
    TWCR = twcr;		/* clear int to start transmission */
    while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
    switch ((twst = TW_STATUS)) {
    case TW_MR_DATA_NACK:
      len = 0;		/* force end of loop */
      /* FALLTHROUGH */
    case TW_MR_DATA_ACK:
      *buf++ = TWDR;
      rv++;
      if(twst == TW_MR_DATA_NACK) goto quit;
      break;

    default:
      goto error;
    }
  }

 error:
  rv = -1;

 quit:
  /* Note [14] */
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN); /* send stop condition */

  return rv;
}

/*
 * Write "len" bytes into EEPROM starting at "eeaddr" from "buf".
 *
 * This is a bit simpler than the previous function since both, the
 * address and the data bytes will be transfered in master transmitter
 * mode, thus no reselection of the device is necessary.  However, the
 * EEPROMs are only capable of writing one "page" simultaneously, so
 * care must be taken to not cross a page boundary within one write
 * cycle.  The amount of data one page consists of varies from
 * manufacturer to manufacturer: some vendors only use 8-byte pages
 * for the smaller devices, and 16-byte pages for the larger devices,
 * while other vendors generally use 16-byte pages.  We thus use the
 * smallest common denominator of 8 bytes per page, declared by the
 * macro EEPROM_PAGE_SIZE above.
 *
 * The function simply returns after writing one page, returning the
 * actual number of data byte written.  It is up to the caller to
 * re-invoke it in order to write further data.
 */
uint16_t
ee24xx_write_page(uint16_t eeaddr, uint8_t *buf, uint16_t len)
{
  uint8_t sla, n = 0;
  uint16_t rv = 0;
  uint16_t endaddr;

  if (eeaddr + len <= (eeaddr | (EEPROM_PAGE_SIZE - 1)))
    endaddr = eeaddr + len;
  else
    endaddr = (eeaddr | (EEPROM_PAGE_SIZE - 1)) + 1;
  len = endaddr - eeaddr;

#ifndef WORD_ADDRESS_16BIT
  /* patch high bits of EEPROM address into SLA */
  sla = TWI_SLA_24CXX | (((eeaddr >> 8) & 0x07) << 1);
#else
  /* 16-bit address devices need only TWI Device Address */
  sla = TWI_SLA_24CXX;
#endif

 restart:
  if (n++ >= MAX_ITER)
    return -1;
 begin:

  /* Note [15] */
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send start condition */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS)) {
  case TW_REP_START:		/* OK, but should not happen */
  case TW_START:
    break;

  case TW_MT_ARB_LOST:
    goto begin;

  default:
    return -1;		/* error: not in start condition */
    /* NB: do /not/ send stop condition */
  }

  /* send SLA+W */
  TWDR = sla | TW_WRITE;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS)) {
  case TW_MT_SLA_ACK:
    break;

  case TW_MT_SLA_NACK:	/* nack during select: device busy writing */
    goto restart;

  case TW_MT_ARB_LOST:	/* re-arbitrate */
    goto begin;

  default:
    goto error;		/* must send stop condition */
  }

#ifdef WORD_ADDRESS_16BIT
  TWDR = (eeaddr>>8);		/* 16 bit word address device, send high 8 bits of addr */
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS)) {
  case TW_MT_DATA_ACK:
    break;

  case TW_MT_DATA_NACK:
    goto quit;

  case TW_MT_ARB_LOST:
    goto begin;

  default:
    goto error;		/* must send stop condition */
  }
#endif

  TWDR = eeaddr;		/* low 8 bits of addr */
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS)) {
  case TW_MT_DATA_ACK:
    break;

  case TW_MT_DATA_NACK:
    goto quit;

  case TW_MT_ARB_LOST:
    goto begin;

  default:
    goto error;		/* must send stop condition */
  }

  for (; len > 0; len--) {
    TWDR = *buf++;
    TWCR = _BV(TWINT) | _BV(TWEN); /* start transmission */
    while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
    switch ((twst = TW_STATUS)) {
    case TW_MT_DATA_NACK:
      goto error;		/* device write protected -- Note [16] */

    case TW_MT_DATA_ACK:
      rv++;
      break;

    default:
      goto error;
    }
  }

 error:
  rv = -1;

 quit:
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN); /* send stop condition */

  return rv;
}

/*
 * Wrapper around ee24xx_write_page() that repeats calling this
 * function until either an error has been returned, or all bytes
 * have been written.
 */
uint16_t
ee24xx_write_bytes(uint16_t eeaddr, uint8_t *buf, uint16_t len)
{
  uint16_t rv, total;

  assert(0 == (len%4));

  total = 0;
  do {
#if DEBUG
    printf("Calling ee24xx_write_page(%d, %d, %p)",
	   eeaddr, len, buf);
#endif
    rv = ee24xx_write_page(eeaddr, buf, len);
#if DEBUG
    printf(" => %d\n", rv);
#endif
    if (rv == -1)
      return -1;
    eeaddr += rv;
    len -= rv;
    buf += rv;
    total += rv;
    LCD_POS(1, 12);
    LCD_PUT_UINT16X(len);
  } while (len > 0);

  return total;
}
