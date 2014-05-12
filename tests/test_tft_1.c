#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#define assert(...)

#include "font.c"
#include "spi.c"
#include "lcd.c"

/*
 2012 Copyright (c) Seeed Technology Inc.

 Authors: Albert.Miao & Loovee, 
 Visweswara R (with initializtion code from TFT vendor)

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/
#ifndef TFTv2_h
#define TFTv2_h

#if defined(ARDUINO) && ARDUINO >= 100
#define SEEEDUINO
#include <Arduino.h>
#else
//#include <WProgram.h>
#endif
#include <avr/pgmspace.h>

#include <spi.h>

//Basic Colors
#define RED		0xf800
#define GREEN	0x07e0
#define BLUE	0x001f
#define BLACK	0x0000
#define YELLOW	0xffe0
#define WHITE	0xffff

//Other Colors
#define CYAN		0x07ff	
#define BRIGHT_RED	0xf810	
#define GRAY1		0x8410  
#define GRAY2		0x4208  

//TFT resolution 240*320
#define MIN_X	0
#define MIN_Y	0
#define MAX_X	239
#define MAX_Y	319

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

#define TFT_CS_LOW  {DDRE |= 0x08;PORTE &=~ 0x08;}
#define TFT_CS_HIGH {DDRE |= 0x08;PORTE |=  0x08;}
#define TFT_DC_LOW  {DDRH |= 0x08;PORTH &=~ 0x08;}
#define TFT_DC_HIGH {DDRH |= 0x08;PORTH |=  0x08;}
#define TFT_BL_OFF  {DDRH |= 0x10;PORTH &=~ 0x10;}
#define TFT_BL_ON   {DDRH |= 0x10;PORTH |=  0x10;}

#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 54   // can be a digital pin, this is A0
#define XP 57   // can be a digital pin, this is A3

#elif defined(__AVR_ATmega32U4__)

#define TFT_CS_LOW  {DDRC |= 0x40;PORTC &=~ 0x40;}
#define TFT_CS_HIGH {DDRC |= 0x40;PORTC |=  0x40;}
#define TFT_DC_LOW  {DDRD |= 0x80;PORTD &=~ 0x80;}
#define TFT_DC_HIGH {DDRD |= 0x80;PORTD |=  0x80;}
#define TFT_BL_OFF  {DDRE |= 0x40;PORTE &=~ 0x40;}
#define TFT_BL_ON   {DDRE |= 0x40;PORTE |=  0x40;}

#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 18   // can be a digital pin, this is A0
#define XP 21   // can be a digital pin, this is A3

#else
#warning "compiling correct code"
#define TFT_RST_LOW  {DDRB |= 0x1;PORTB &= ~0x1;} //Added by Vassilis Serasidis (18 Oct 2013)
#define TFT_RST_HIGH {DDRB |= 0x1;PORTB |=  0x1;} //Added by Vassilis Serasidis (18 Oct 2013)
#define TFT_CS_LOW  {DDRB |= 0x8;PORTB &= ~0x8;}
#define TFT_CS_HIGH {DDRB |= 0x8;PORTB |=  0x8;}
#define TFT_DC_LOW  {DDRB |= 0x2;PORTB &= ~0x2;}
#define TFT_DC_HIGH {DDRB |= 0x2;PORTB |=  0x2;}
#define TFT_BL_OFF  {DDRD |= 0x10;PORTD &= ~0x10;}
#define TFT_BL_ON   {DDRD |= 0x10;PORTD |=  0x10;}

#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 14   // can be a digital pin, this is A0
#define XP 17   // can be a digital pin, this is A3

#endif

#define TS_MINX 116*2
#define TS_MAXX 890*2
#define TS_MINY 83*2
#define TS_MAXY 913*2

#ifndef INT8U
#define INT8U unsigned char
#endif
#ifndef INT16U
#define INT16U unsigned int
#endif

//extern INT8U simpleFont[][8];

void TFTinit (void);
void setCol(INT16U StartCol,INT16U EndCol);
void setPage(INT16U StartPage,INT16U EndPage);
void setXY(INT16U poX, INT16U poY);
void setPixel(INT16U poX, INT16U poY,INT16U color);
void sendCMD(INT8U index);
void WRITE_Package(INT16U *data,INT8U howmany);
void WRITE_DATA(INT8U data);
void sendData(INT16U data);
INT8U Read_Register(INT8U Addr,INT8U xParameter);
void fillScreen(INT16U XL,INT16U XR,INT16U YU,INT16U YD,INT16U color);
void fillScreen1(void);
INT8U readID(void);
	
void drawChar(INT8U ascii,INT16U poX, INT16U poY,INT16U size, INT16U fgcolor);
void drawString(char *string,INT16U poX, INT16U poY,INT16U size,INT16U fgcolor);
void fillRectangle(INT16U poX, INT16U poY, INT16U length, INT16U width, INT16U color);
	
void drawLine(INT16U x0,INT16U y0,INT16U x1,INT16U y1,INT16U color);
void drawVerticalLine(INT16U poX, INT16U poY,INT16U length,INT16U color);
void drawHorizontalLine(INT16U poX, INT16U poY,INT16U length,INT16U color);
void drawRectangle(INT16U poX, INT16U poY, INT16U length,INT16U width,INT16U color);
	
void drawCircle(int poX, int poY, int r,INT16U color);
void fillCircle(int poX, int poY, int r,INT16U color);
	
void drawTraingle(int poX1, int poY1, int poX2, int poY2, int poX3, int poY3, INT16U color);
INT8U drawNumber(long long_num,INT16U poX, INT16U poY,INT16U size,INT16U fgcolor);
INT8U drawFloat(float floatNumber,INT8U decimal,INT16U poX, INT16U poY,INT16U size,INT16U fgcolor);
INT8U drawFloat1(float floatNumber,INT16U poX, INT16U poY,INT16U size,INT16U fgcolor);

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/

/*
 2012 Copyright (c) Seeed Technology Inc.

 Authors: Albert.Miao & Loovee,
 Visweswara R (with initializtion code from TFT vendor)

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc.,51 Franklin St,Fifth Floor, Boston, MA 02110-1301 USA

*/

#include <util/delay.h>
//#include <TFTv2.h>
#include <spi.h>
#define FONT_SPACE 6
#define FONT_X 8
#define FONT_Y 8

void sendCMD(INT8U index)
{
    TFT_DC_LOW;
    TFT_CS_LOW;
    SPI_transmit(index);
    TFT_CS_HIGH;
}

void WRITE_DATA(INT8U data)
{
    TFT_DC_HIGH;
    TFT_CS_LOW;
    SPI_transmit(data);
    TFT_CS_HIGH;
}

void sendData(INT16U data)
{
    INT8U data1 = data>>8;
    INT8U data2 = data&0xff;
    TFT_DC_HIGH;
    TFT_CS_LOW;
    SPI_transmit(data1);
    SPI_transmit(data2);
    TFT_CS_HIGH;
}

void WRITE_Package(INT16U *data, INT8U howmany)
{
    INT16U    data1 = 0;
    INT8U   data2 = 0;

    TFT_DC_HIGH;
    TFT_CS_LOW;
    INT8U count=0;
    for(count=0;count<howmany;count++)
    {
        data1 = data[count]>>8;
        data2 = data[count]&0xff;
        SPI_transmit(data1);
        SPI_transmit(data2);
    }
    TFT_CS_HIGH;
}

INT8U Read_Register(INT8U Addr, INT8U xParameter)
{
    INT8U data=0;
    sendCMD(0xd9);                                                      /* ext command                  */
    WRITE_DATA(0x10+xParameter);                                        /* 0x11 is the first Parameter  */
    TFT_DC_LOW;
    TFT_CS_LOW;
    SPI_transmit(Addr);
    TFT_DC_HIGH;
    data = SPI_transmit(0);
    TFT_CS_HIGH;
    return data;
}

void TFTinit (void)
{
	TFT_RST_LOW; //Added by Vassilis Serasidis (18 Oct 2013)
	_delay_ms(200);  //Added by Vassilis Serasidis (18 Oct 2013)
	TFT_RST_HIGH; //Added by Vassilis Serasidis (18 Oct 2013)
    spi_init(); 
    TFT_CS_HIGH;
    TFT_DC_HIGH;
    INT8U i=0, TFTDriver=0;
    for(i=0;i<3;i++)
    {
        TFTDriver = readID();
    }
    _delay_ms(500);
    sendCMD(0x01);
    _delay_ms(200);

    sendCMD(0xCF);
    WRITE_DATA(0x00);
    WRITE_DATA(0x8B);
    WRITE_DATA(0X30);

    sendCMD(0xED);
    WRITE_DATA(0x67);
    WRITE_DATA(0x03);
    WRITE_DATA(0X12);
    WRITE_DATA(0X81);

    sendCMD(0xE8);
    WRITE_DATA(0x85);
    WRITE_DATA(0x10);
    WRITE_DATA(0x7A);

    sendCMD(0xCB);
    WRITE_DATA(0x39);
    WRITE_DATA(0x2C);
    WRITE_DATA(0x00);
    WRITE_DATA(0x34);
    WRITE_DATA(0x02);

    sendCMD(0xF7);
    WRITE_DATA(0x20);

    sendCMD(0xEA);
    WRITE_DATA(0x00);
    WRITE_DATA(0x00);

    sendCMD(0xC0);                                                      /* Power control                */
    WRITE_DATA(0x1B);                                                   /* VRH[5:0]                     */

    sendCMD(0xC1);                                                      /* Power control                */
    WRITE_DATA(0x10);                                                   /* SAP[2:0];BT[3:0]             */

    sendCMD(0xC5);                                                      /* VCM control                  */
    WRITE_DATA(0x3F);
    WRITE_DATA(0x3C);

    sendCMD(0xC7);                                                      /* VCM control2                 */
    WRITE_DATA(0XB7);

    sendCMD(0x36);                                                      /* Memory Access Control        */
    WRITE_DATA(0x08);

    sendCMD(0x3A);
    WRITE_DATA(0x55);

    sendCMD(0xB1);
    WRITE_DATA(0x00);
    WRITE_DATA(0x1B);

    sendCMD(0xB6);                                                      /* Display Function Control     */
    WRITE_DATA(0x0A);
    WRITE_DATA(0xA2);


    sendCMD(0xF2);                                                      /* 3Gamma Function Disable      */
    WRITE_DATA(0x00);

    sendCMD(0x26);                                                      /* Gamma curve selected         */
    WRITE_DATA(0x01);

    sendCMD(0xE0);                                                      /* Set Gamma                    */
    WRITE_DATA(0x0F);
    WRITE_DATA(0x2A);
    WRITE_DATA(0x28);
    WRITE_DATA(0x08);
    WRITE_DATA(0x0E);
    WRITE_DATA(0x08);
    WRITE_DATA(0x54);
    WRITE_DATA(0XA9);
    WRITE_DATA(0x43);
    WRITE_DATA(0x0A);
    WRITE_DATA(0x0F);
    WRITE_DATA(0x00);
    WRITE_DATA(0x00);
    WRITE_DATA(0x00);
    WRITE_DATA(0x00);

    sendCMD(0XE1);                                                      /* Set Gamma                    */
    WRITE_DATA(0x00);
    WRITE_DATA(0x15);
    WRITE_DATA(0x17);
    WRITE_DATA(0x07);
    WRITE_DATA(0x11);
    WRITE_DATA(0x06);
    WRITE_DATA(0x2B);
    WRITE_DATA(0x56);
    WRITE_DATA(0x3C);
    WRITE_DATA(0x05);
    WRITE_DATA(0x10);
    WRITE_DATA(0x0F);
    WRITE_DATA(0x3F);
    WRITE_DATA(0x3F);
    WRITE_DATA(0x0F);

    sendCMD(0x11);                                                      /* Exit Sleep                   */
    _delay_ms(120);
    sendCMD(0x29);                                                      /* Display on                   */
    fillScreen1();
}

INT8U readID(void)
{
    INT8U i=0;
    INT8U data[3] ;
    INT8U ID[3] = {0x00, 0x93, 0x41};
    INT8U ToF=1;
    for(i=0;i<3;i++)
    {
        data[i]=Read_Register(0xd3,i+1);
        if(data[i] != ID[i])
        {
            ToF=0;
        }
    }
    if(!ToF)                                                            /* data!=ID                     */
    {
//        Serial.print("Read TFT ID failed, ID should be 0x09341, but read ID = 0x");
//        for(i=0;i<3;i++)
//        {
//            Serial.print(data[i],HEX);
//        }
//        Serial.println();
      ERROR("Read TFT ID failed, ID should be 0x09341, but read ID = 0x");
    }
    return ToF;
}

void setCol(INT16U StartCol,INT16U EndCol)
{
    sendCMD(0x2A);                                                      /* Column Command address       */
    sendData(StartCol);
    sendData(EndCol);
}

void setPage(INT16U StartPage,INT16U EndPage)
{
    sendCMD(0x2B);                                                      /* Column Command address       */
    sendData(StartPage);
    sendData(EndPage);
}

#define constrain(X, MinX, MaxX) \
  ( (X < MinX) ? MinX : ((X > MaxX) ? MaxX : X) )

void fillScreen(INT16U XL, INT16U XR, INT16U YU, INT16U YD, INT16U color)
{
    unsigned long  XY=0;
    unsigned long i=0;

    if(XL > XR)
    {
        XL = XL^XR;
        XR = XL^XR;
        XL = XL^XR;
    }
    if(YU > YD)
    {
        YU = YU^YD;
        YD = YU^YD;
        YU = YU^YD;
    }
    XL = constrain(XL, MIN_X,MAX_X);
    XR = constrain(XR, MIN_X,MAX_X);
    YU = constrain(YU, MIN_Y,MAX_Y);
    YD = constrain(YD, MIN_Y,MAX_Y);

    XY = (XR-XL+1);
    XY = XY*(YD-YU+1);

    setCol(XL,XR);
    setPage(YU, YD);
    sendCMD(0x2c);                                                  /* start to write to display ra */
                                                                        /* m                            */

    TFT_DC_HIGH;
    TFT_CS_LOW;

    INT8U Hcolor = color>>8;
    INT8U Lcolor = color&0xff;
    for(i=0; i < XY; i++)
    {
        SPI_transmit(Hcolor);
        SPI_transmit(Lcolor);
    }

    TFT_CS_HIGH;
}

void fillScreen1(void)
{
    setCol(0, 239);
    setPage(0, 319);
    sendCMD(0x2c);                                                  /* start to write to display ra */
                                                                        /* m                            */

    TFT_DC_HIGH;
    TFT_CS_LOW;
    for(INT16U i=0; i<38400; i++)
    {
        SPI_transmit(0);
        SPI_transmit(0);
        SPI_transmit(0);
        SPI_transmit(0);
    }
    TFT_CS_HIGH;
}


void setXY(INT16U poX, INT16U poY)
{
    setCol(poX, poX);
    setPage(poY, poY);
    sendCMD(0x2c);
}

void setPixel(INT16U poX, INT16U poY,INT16U color)
{
    setXY(poX, poY);
    sendData(color);
}

void drawChar( INT8U ascii, INT16U poX, INT16U poY,INT16U size, INT16U fgcolor)
{
    if((ascii>=32)&&(ascii<=127))
    {
        ;
    }
    else
    {
        ascii = '?'-32;
    }
    for (int i =0; i<FONT_X; i++ ) {
        INT8U temp = pgm_read_byte(&simpleFont[ascii-0x20][i]);
        for(INT8U f=0;f<8;f++)
        {
            if((temp>>f)&0x01)
            {
                fillRectangle(poX+i*size, poY+f*size, size, size, fgcolor);
            }

        }

    }
}

void drawString(char *string,INT16U poX, INT16U poY, INT16U size,INT16U fgcolor)
{
    while(*string)
    {
        drawChar(*string, poX, poY, size, fgcolor);
        *string++;

        if(poX < MAX_X)
        {
            poX += FONT_SPACE*size;                                     /* Move cursor right            */
        }
    }
}

//fillRectangle(poX+i*size, poY+f*size, size, size, fgcolor);
void fillRectangle(INT16U poX, INT16U poY, INT16U length, INT16U width, INT16U color)
{
    fillScreen(poX, poX+length, poY, poY+width, color);
}

void  drawHorizontalLine( INT16U poX, INT16U poY,
INT16U length,INT16U color)
{
    setCol(poX,poX + length);
    setPage(poY,poY);
    sendCMD(0x2c);
    for(INT16U i=0; i<length; i++)
    sendData(color);
}

#define abs(x) ((x<0) ? -x : x)

void drawLine( INT16U x0,INT16U y0,INT16U x1, INT16U y1,INT16U color)
{

    int x = x1-x0;
    int y = y1-y0;
    int dx = abs(x), sx = x0<x1 ? 1 : -1;
    int dy = -abs(y), sy = y0<y1 ? 1 : -1;
    int err = dx+dy, e2;                                                /* error value e_xy             */
    for (;;){                                                           /* loop                         */
        setPixel(x0,y0,color);
        e2 = 2*err;
        if (e2 >= dy) {                                                 /* e_xy+e_x > 0                 */
            if (x0 == x1) break;
            err += dy; x0 += sx;
        }
        if (e2 <= dx) {                                                 /* e_xy+e_y < 0                 */
            if (y0 == y1) break;
            err += dx; y0 += sy;
        }
    }

}

void drawVerticalLine( INT16U poX, INT16U poY, INT16U length,INT16U color)
{
    setCol(poX,poX);
    setPage(poY,poY+length);
    sendCMD(0x2c);
    for(INT16U i=0; i<length; i++)
    sendData(color);
}

void drawRectangle(INT16U poX, INT16U poY, INT16U length, INT16U width,INT16U color)
{
    drawHorizontalLine(poX, poY, length, color);
    drawHorizontalLine(poX, poY+width, length, color);
    drawVerticalLine(poX, poY, width,color);
    drawVerticalLine(poX + length, poY, width,color);

}

void drawCircle(int poX, int poY, int r,INT16U color)
{
    int x = -r, y = 0, err = 2-2*r, e2;
    do {
        setPixel(poX-x, poY+y,color);
        setPixel(poX+x, poY+y,color);
        setPixel(poX+x, poY-y,color);
        setPixel(poX-x, poY-y,color);
        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);
}

void fillCircle(int poX, int poY, int r,INT16U color)
{
    int x = -r, y = 0, err = 2-2*r, e2;
    do {

        drawVerticalLine(poX-x, poY-y, 2*y, color);
        drawVerticalLine(poX+x, poY-y, 2*y, color);

        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);

}

void drawTraingle( int poX1, int poY1, int poX2, int poY2, int poX3, int poY3, INT16U color)
{
    drawLine(poX1, poY1, poX2, poY2,color);
    drawLine(poX1, poY1, poX3, poY3,color);
    drawLine(poX2, poY2, poX3, poY3,color);
}

INT8U drawNumber(long long_num,INT16U poX, INT16U poY,INT16U size,INT16U fgcolor)
{
    INT8U char_buffer[10] = "";
    INT8U i = 0;
    INT8U f = 0;

    if (long_num < 0)
    {
        f=1;
        drawChar('-',poX, poY, size, fgcolor);
        long_num = -long_num;
        if(poX < MAX_X)
        {
            poX += FONT_SPACE*size;                                     /* Move cursor right            */
        }
    }
    else if (long_num == 0)
    {
        f=1;
        drawChar('0',poX, poY, size, fgcolor);
        return f;
        if(poX < MAX_X)
        {
            poX += FONT_SPACE*size;                                     /* Move cursor right            */
        }
    }


    while (long_num > 0)
    {
        char_buffer[i++] = long_num % 10;
        long_num /= 10;
    }

    f = f+i;
    for(; i > 0; i--)
    {
        drawChar('0'+ char_buffer[i - 1],poX, poY, size, fgcolor);
        if(poX < MAX_X)
        {
            poX+=FONT_SPACE*size;                                       /* Move cursor right            */
        }
    }
    return f;
}

INT8U drawFloat(float floatNumber,INT8U decimal,INT16U poX, INT16U poY,INT16U size,INT16U fgcolor)
{
    INT16U temp=0;
    float decy=0.0;
    float rounding = 0.5;
    INT8U f=0;
    if(floatNumber<0.0)
    {
        drawChar('-',poX, poY, size, fgcolor);
        floatNumber = -floatNumber;
        if(poX < MAX_X)
        {
            poX+=FONT_SPACE*size;                                       /* Move cursor right            */
        }
        f =1;
    }
    for (INT8U i=0; i<decimal; ++i)
    {
        rounding /= 10.0;
    }
    floatNumber += rounding;

    temp = (INT16U)floatNumber;
    INT8U howlong=drawNumber(temp,poX, poY, size, fgcolor);
    f += howlong;
    if((poX+8*size*howlong) < MAX_X)
    {
        poX+=FONT_SPACE*size*howlong;                                   /* Move cursor right            */
    }

    if(decimal>0)
    {
        drawChar('.',poX, poY, size, fgcolor);
        if(poX < MAX_X)
        {
            poX+=FONT_SPACE*size;                                       /* Move cursor right            */
        }
        f +=1;
    }
    decy = floatNumber-temp;                                            /* decimal part,  4             */
    for(INT8U i=0;i<decimal;i++)                                      
    {
        decy *=10;                                                      /* for the next decimal         */
        temp = decy;                                                    /* get the decimal              */
        drawNumber(temp,poX, poY, size, fgcolor);
        floatNumber = -floatNumber;
        if(poX < MAX_X)
        {
            poX+=FONT_SPACE*size;                                       /* Move cursor right            */
        }
        decy -= temp;
    }
    f +=decimal;
    return f;
}

INT8U drawFloat1(float floatNumber,INT16U poX, INT16U poY,INT16U size,INT16U fgcolor)
{
    INT8U decimal=2;
    INT16U temp=0;
    float decy=0.0;
    float rounding = 0.5;
    INT8U f=0;
    if(floatNumber<0.0)                                                 /* floatNumber < 0              */
    {
        drawChar('-',poX, poY, size, fgcolor);                          /* add a '-'                    */
        floatNumber = -floatNumber;
        if(poX < MAX_X)
        {
            poX+=FONT_SPACE*size;                                       /* Move cursor right            */
        }
        f =1;
    }
    for (INT8U i=0; i<decimal; ++i)
    {
        rounding /= 10.0;
    }
    floatNumber += rounding;

    temp = (INT16U)floatNumber;
    INT8U howlong=drawNumber(temp,poX, poY, size, fgcolor);
    f += howlong;
    if((poX+8*size*howlong) < MAX_X)
    {
        poX+=FONT_SPACE*size*howlong;                                   /* Move cursor right            */
    }


    if(decimal>0)
    {
        drawChar('.',poX, poY, size, fgcolor);
        if(poX < MAX_X)
        {
            poX += FONT_SPACE*size;                                     /* Move cursor right            */
        }
        f +=1;
    }
    decy = floatNumber-temp;                                            /* decimal part,                */
    for(INT8U i=0;i<decimal;i++)
    {
        decy *=10;                                                      /* for the next decimal         */
        temp = decy;                                                    /* get the decimal              */
        drawNumber(temp,poX, poY, size, fgcolor);
        floatNumber = -floatNumber;
        if(poX < MAX_X)
        {
            poX += FONT_SPACE*size;                                     /* Move cursor right            */
        }
        decy -= temp;
    }
    f += decimal;
    return f;
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/

#if 1
int
main()
{
  _delay_ms(1000);
  LCD_init();
  _delay_ms(5000);
  LCD_WR_LINE(0, 0, "Hello Naren W2");
  LCD_WR_LINE(1, 0, "Hello Naren W3");
  LCD_refresh();

  DDRB |=  (1<<4) | (1<<5) | (1<<7);
  DDRB &=  ~(1<<6);

  _delay_ms(1000);
  TFTinit();
  TFT_BL_ON;
  _delay_ms(1000);
  do {
    LCD_WR_LINE(1, 0, "What the suck!");
    LCD_refresh();
    drawString("Hello",0,180,3,CYAN);       // draw string: "hello", (0, 180), size: 3, color: CYAN
    drawString("World!!",60,220,4,WHITE);    // draw string: "world!!", (80, 230), size: 4, color: WHITE
  } while (0);

  uint8_t ui1;
  for (ui1=0; ; ui1++) {
    LCD_POS(0, 14);
    LCD_PUT_UINT8X(ui1);
    LCD_refresh();
    _delay_ms(100);
  }

}
#else
int
main()
{
  _delay_ms(1000);
  LCD_init();
  LCD_WR_LINE(1, 0, "Hello Naren W3");
  LCD_refresh();
  _delay_ms(2000);

  DDRB |=  (1<<4) | (1<<5) | (1<<7);
  DDRB &=  ~(1<<6);

  TFT_RST_LOW; //Added by Vassilis Serasidis (18 Oct 2013)
  _delay_ms(200);  //Added by Vassilis Serasidis (18 Oct 2013)
  TFT_RST_HIGH; //Added by Vassilis Serasidis (18 Oct 2013)
  spi_init(); 
  TFT_CS_HIGH;
  TFT_DC_HIGH;

  TFT_BL_ON;
  uint8_t i, ui1;
  while (1) {
  for(i=0;i<3;i++) {
    sendCMD(0xd9);                                                      /* ext command                  */
    WRITE_DATA(0x10+i+1);                                        /* 0x11 is the first Parameter  */
    TFT_DC_LOW;
    TFT_CS_LOW;
    SPI_transmit(0xd3);
    TFT_DC_HIGH;
    ui1 = SPI_transmit(0);
    TFT_CS_HIGH;

    LCD_POS(1, 14);
    LCD_PUT_UINT8X(ui1);
    LCD_refresh();
    _delay_ms(1000);
  }
  }
}
#endif
