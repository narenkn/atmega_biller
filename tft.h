#ifndef TFT_H
#define TFT_H

#define TFT_RST_LOW          PORTB &=  ~0x1
#define TFT_RST_HIGH         PORTB |=   0x1
#define TFT_CS_LOW           PORTB &=  ~0x8
#define TFT_CS_HIGH          PORTB |=   0x8
#define TFT_DC_LOW           PORTB &=  ~0x2
#define TFT_DC_HIGH          PORTB |=   0x2
#define TFT_BL_OFF           PORTD &= ~0x10
#define TFT_BL_ON            PORTD |=  0x10

#define TFT_WIDTH       240
#define TFT_HEIGHT      320

void TftInit(void);
void TftClear(uint16_t Color);
void TftAddressSet(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void TftDrawPoint(uint16_t x, uint16_t y);
void TftDrawPointBig(uint16_t x, uint16_t y);
void TftDrawCircle(uint16_t x0, uint16_t y0, uint8_t r);
void TftDrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void TftDrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void TftFill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color);
void TftShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t mode);
void TftShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len);
void TftShow2Num(uint16_t x, uint16_t y, uint16_t num, uint8_t len);
void TftShowString(uint16_t x, uint16_t y, const uint8_t *p);

#define WHITE         	 0xFFFF
#define BLACK         	 0x0000
#define BLUE         	 0x001F
#define BRED             0xF81F
#define GRED 		 0xFFE0
#define GBLUE		 0x07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 		 0xBC40
#define BRRED 		 0xFC07
#define GRAY  		 0x8430


#define DARKBLUE      	 0x01CF	
#define LIGHTBLUE      	 0x7D7C	
#define GRAYBLUE       	 0x5458


#define LIGHTGREEN     	 0x841F
#define LGRAY 		 0xC618

#define LGRAYBLUE        0xA651
#define LBBLUE           0x2B12

extern uint16_t BACK_COLOR, POINT_COLOR;

#endif
