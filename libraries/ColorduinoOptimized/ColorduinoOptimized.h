/*
  ColorduinoOptimized - Optimized Colorduino Library for Atmega328p + DM163 LED Driver
  Last modified by Jacky at (UTC+8)23:35, June 11th, 2018
  Copyright (c) 2018 Jacky Zhang <creative.zhang@gmail.com>
  
  based on C++ code by Sam C. Lin
  Copyright (c) 2011-2016 Sam C. Lin <lincomatic@hotmail.com>
  
  based on C code by zzy@iteadstudio
  Copyright (c) 2010 zzy@IteadStudio.  All right reserved.
  
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
#ifndef _COLORDUINO_OPT_
#define _COLORDUINO_OPT_

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#include "pins_arduino.h"
#endif

/* #include <avr/pgmspace.h> */
#include <avr/io.h>
#include <avr/interrupt.h>

/*
   DM163 LED Driver
   8bit color and 6bit white balance correction
   input data: 6'b001111 + 8'b00011111
   output current: I-out * (15/64) * (31/256)
*/
#define ColorduinoBitsPerColor 8
#define ColorduinoBitsPerCorrection 6

#define ColorduinoScreenWidth 8
#define ColorduinoScreenHeight 8

/**

    define the IO

*/
#define RST_BIT 0x04 //PC2  //A2 for PinMode func
#define LAT_BIT 0x02 //PC1  //A1
#define SLB_BIT 0x01 //PC0  //A0
#define SCL_BIT 0x40 //PD6  0x40=0b01000000  //PD0~PD7 0~7 for PinMode func  //6 for PinMode func PD6
#define SDA_BIT 0x80 //PD7  0x80=0b10000000  //7 for PinMode func PD7

#define RST PORTC
#define LAT PORTC
#define SLB PORTC
#define SDA PORTD
#define SCL PORTD

/*
   [line] means vertical line of Colorduino
  #define open_line0  {PORTB=0b01;} // PB0
  #define open_line1  {PORTB=0b10;} // PB1
  #define open_line2  {PORTB=B100;} // PB2
  #define open_line3  {PORTB=0x08;} // PB3
  #define open_line4  {PORTB=0x10;} // PB4
  #define open_line5  {PORTB=0x20;} // PB5
  #define open_line6  {PORTD=0x08;} // PD3
  #define open_line7  {PORTD=0x10;} // PD4
  #define close_all_lines {PORTD=0x00;PORTB=0x00;}
*/
#define open_line0  {PORTD=0x10;} // PD4  //4 for PinMode func
#define open_line1  {PORTD=0x08;} // PD3  //3 for PinMode func
#define open_line2  {PORTB=0x20;} // PB5  //PB0~PB5 8~13 for PinMode func  //13 for PinMode func PB5
#define open_line3  {PORTB=0x10;} // PB4  //12 for PinMode func PB4
#define open_line4  {PORTB=0x08;} // PB3  //11 for PinMode func PB3
#define open_line5  {PORTB=B100;} // PB2  //10 for PinMode func PB2
#define open_line6  {PORTB=0b10;} // PB1  //9  for PinMode func PB1
#define open_line7  {PORTB=0b01;} // PB0  //8  for PinMode func PB0
#define close_all_lines {PORTD=0x00;PORTB=0x00;}

#define LED_RST_SET RST|=RST_BIT // PORTC = PORTC | B00000100 // turn on PC2 // set LED_RST = High
#define LED_RST_CLR RST&=~RST_BIT // PORTC = PORTC & ~(B00000100) = PORTC & B11111011 // turn off PC2
#define LED_SDA_SET SDA|=SDA_BIT
#define LED_SDA_CLR SDA&=~SDA_BIT
#define LED_SCL_SET SCL|=SCL_BIT
#define LED_SCL_CLR SCL&=~SCL_BIT
#define LED_LAT_SET LAT|=LAT_BIT
#define LED_LAT_CLR LAT&=~LAT_BIT
#define LED_SLB_SET SLB|=SLB_BIT
#define LED_SLB_CLR SLB&=~SLB_BIT

typedef struct pixelRGB {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} PixelRGB;

class ColorduinoObject {
  public:

    // Members of Class
    PixelRGB frameBuffer0[ColorduinoScreenWidth * ColorduinoScreenHeight];
    PixelRGB frameBuffer1[ColorduinoScreenWidth * ColorduinoScreenHeight];
    PixelRGB *curDrawFrame; // Frame that will be shown on screen by [run] method
    PixelRGB *curWriteFrame; // Frame in cache will be swapped by [FlipPage] method
    unsigned char line = 0;

    // Constructor
    ColorduinoObject();

    // Hardware Initial Function
    void _IO_Init();
    void _LED_Init();
    void _TC2_Init();
    void Init();

    // ISR Call to Maintain Refreshing Frames
    void run();
    
    // Functions
    void LED_Delay(unsigned char);
    void SetWhiteBal(unsigned char *);
    void open_line(unsigned char);
    void FlipPage(byte method = 0);

    // Drawing Functions
    PixelRGB * GetPixel(char, char);
    PixelRGB * GetDrawPixel(char, char);
    void SetPixel(char, char, unsigned char, unsigned char, unsigned char);
    void SetPixel(char, char, const PixelRGB &);
    void ColorFill(unsigned char, unsigned char, unsigned char);

};

extern ColorduinoObject Colorduino;

#endif // _COLORDUINO_OPT_

