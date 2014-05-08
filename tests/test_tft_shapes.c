// Draw Boxes - Demonstrate drawRectangle and fillRectangle

#include <stdint.h>
#include <font.c>
#include <TFTv2.cpp>
#include <spi.c>

void setup()
{
    TFT_BL_ON;                                          // turn on the background light
    Tft.TFTinit();                                      // init TFT library
}

void loop()
{
    for(int r=0;r<115;r=r+2)                            //set r : 0--115
    {
        Tft.drawCircle(119,160,r,random(0xFFFF));       //draw circle, center:(119, 160), color: random
    }
    _delay_ms(10);
}

int
main()
{
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
