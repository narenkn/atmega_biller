#define ERROR(...)
#include <TFTv2.cpp>
#include <spi.h>

// pin definition for the Uno
#define cs   10
#define dc   9
#define rst  8  
// CS pin 3 of tft conected in arduino pin 10
// RST pin 4 of tft conected in arduino pin 8 
// D/C pin 5 of tft conected in arduino pin 9 
// MOSI pin 6 of tft conected in arduino pin 11
// SCLK pin 7 of tft conected in arduino pin 13

// create an instance of the library
TFT TFTscreen = TFT(cs, dc, rst);
//TFT TFTscreen = TFT(cs, rst, dc); //not work
//TFT TFTscreen = TFT(dc, cs, rst); //not work
//TFT TFTscreen = TFT(dc, rst, cs); //not work
//TFT TFTscreen = TFT(rst, cs, dc); //not work
//TFT TFTscreen = TFT(rst, dc, cs); //not work

// char array to print to the screen
char sensorPrintout[4];

void setup() {
  
  // Put this line at the beginning of every sketch that uses the GLCD:
  TFTscreen.begin();

  // clear the screen with a black background
  TFTscreen.background(0, 0, 0);
  
  // write the static text to the screen
  // set the font color to white
  TFTscreen.stroke(255,255,255);
  // set the font size
  TFTscreen.setTextSize(2);
  // write the text to the top left corner of the screen
  TFTscreen.text("Sensor Value :\n ",0,0);
  // ste the font size very large for the loop
  TFTscreen.setTextSize(5);
}

void loop() {

  // Read the value of the sensor on A0
  String sensorVal = String(analogRead(A0));
 
  // convert the reading to a char array
  sensorVal.toCharArray(sensorPrintout, 4);

  // set the font color
  TFTscreen.stroke(255,255,255);
  // print the sensor value
  TFTscreen.text(sensorPrintout, 0, 20);
  // wait for a moment
  delay(250);
  // erase the text you just wrote
  TFTscreen.stroke(0,0,0);
  TFTscreen.text(sensorPrintout, 0, 20);
}
