//********************************************************
// ******** 4X4 MATRIX KEY-BOARD INTERFACING ***********
//********************************************************
//Controller:	ATmega16 (Crystal: 16 Mhz)
//Compiler:		winAVR (AVRStudio)
//Author:		CC Dharmani, Chennai (India)
//              www.dharmanitech.com
//Date:			June 2009
//********************************************************

#include <avr/io.h>
#include <util/delay.h>

#define KB_PORT_OUT 	PORTA
#define KB_PORT_IN	PINC

//************************
void
port_init(void)
{
  DDRC  = ~0x3C;	//Key-board port, higer nibble - input, lower nibble - output
  PORTC = 0x3C;		//pull-up enabled for higher nibble
  DDRA  = 0xF0;
  PORTA = 0xF0;
}


//call this routine to initialize all peripherals
void
init_devices(void)
{
  port_init();
 
  MCUCR = 0x00;
  TIMSK = 0x00; //timer interrupt sources
} 

//****************** MAIN FUNCTION *******************

int
main(void)
{
  unsigned char upperNibble, keyCode, keyPressed, i;
  init_devices();
  DDRB |= 1;

  while(1) {
    upperNibble = 0xff;
		
    for(i=0; i<4; i++) {
      _delay_ms(1);
      KB_PORT_OUT = ~(0x10 << i);
      _delay_ms(1);  		  	 		  //delay for port o/p settling
      upperNibble = KB_PORT_IN | 0xC3;
		 
      if (upperNibble != 0xff) {
	_delay_ms(20); 		  		 //key debouncing delay
	upperNibble = KB_PORT_IN | 0xC3;
	if(upperNibble == 0xff) goto OUT;
		   
//	keyCode = (upperNibble & 0x3C) | (0x0f & ~(0x01 << i));
		   
	while (upperNibble != 0xff)
	  upperNibble = KB_PORT_IN | 0xC3;
		   
	_delay_ms(20);   			   //key debouncing delay
		   
	PORTB = 1;
			
      OUT:;
      }//end of if
    }//end of for
    _delay_ms(1000);
    PORTB = 0;
    _delay_ms(1000);
  }//end of while(1)	 
	
  return 0;    
}//end of main()


//******** CC DHARMANI  *********  www.dharmanitech.com *********
