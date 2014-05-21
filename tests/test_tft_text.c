/*  draw text's APP
    drawChar(INT8U ascii,INT16U poX, INT16U poY,INT16U size, INT16U fgcolor);
    drawString(char *string,INT16U poX, INT16U poY,INT16U size,INT16U fgcolor);
*/
#define assert(...)
#include <stdint.h>
#include <font.c>
#include <TFTv2.cpp>
#include <spi.c>
#include <lcd.c>

void
setup()
{
    TFT_BL_ON;      // turn on the background light
    Tft.TFTinit();  // init TFT library

    Tft.drawChar('S',0,0,1,RED);                // draw char: 'S', (0, 0), size: 1, color: RED
    Tft.drawChar('E',10,10,2,BLUE);             // draw char: 'E', (10, 10), size: 2, color: BLUE
    Tft.drawChar('E',20,40,3,GREEN);            // draw char: 'E', (20, 40), size: 3, color: GREEN
    Tft.drawChar('E',30,80,4,YELLOW);           // draw char: 'E', (30, 80), size: 4, color: YELLOW
    Tft.drawChar('D',40,120,4,YELLOW);          // draw char: 'D', (40, 120), size: 4, color: YELLOW
    Tft.drawString("Hello",0,180,3,CYAN);       // draw string: "hello", (0, 180), size: 3, color: CYAN
    Tft.drawString("World!!",60,220,4,WHITE);    // draw string: "world!!", (80, 230), size: 4, color: WHITE
}

void loop()
{
  
}

int
main()
{
  uint8_t ui1, ui2;
  _delay_ms(1000);
  TFT_RST_HIGH;
  _delay_ms(1000);
  TFT_RST_LOW;
  _delay_ms(5000);
  TFT_RST_HIGH;
  _delay_ms(1000);

  _delay_ms(1000);
  LCD_init();
  LCD_WR_LINE(1, 0, "Hello Hi W3");
  LCD_refresh();
  _delay_ms(2000);

  DDRB |=  (1<<4) | (1<<5) | (1<<7);
  DDRB &=  ~(1<<6);

  TFT_BL_ON;
  spi_init();
//  setup();
  ui1=0; ui2=0;
  while(1) {
    Tft.drawChar('S',ui1,ui2,1,0xF0F0);
    ui1++;
    ui2++;

    LCD_POS(1, 14);
    LCD_PUT_UINT8X(ui1);
    LCD_refresh();
    _delay_ms(1000);
  }
}
