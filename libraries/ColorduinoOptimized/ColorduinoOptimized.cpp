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
#include "ColorduinoOptimized.h"

ColorduinoObject::ColorduinoObject() {
  curWriteFrame = frameBuffer0;
  curDrawFrame = frameBuffer1;
}

void ColorduinoObject::Init() {
  _IO_Init();           //Init IO
  _LED_Init();          //Init LED Hardware
  _TC2_Init();          //Init Timer/Count2
}

void ColorduinoObject::_LED_Init()
{
  LED_RST_SET;
  LED_Delay(1);
  LED_RST_CLR;
  LED_Delay(1);
  LED_RST_SET;
  LED_Delay(1);
  line = 0;
}


void ColorduinoObject::_IO_Init()
{
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  DDRF = 0xff;
  DDRH = 0xff;
  DDRB = 0xff;
  DDRE = 0xff;
  DDRG = 0xff;

  PORTF = 0x00;
  PORTH = 0x00;
  PORTB = 0x00;
  PORTE = 0x00;
  PORTG = 0x00;
#elif defined(__AVR_ATmega328__) || defined(__AVR_ATmega168__)
  DDRD = 0xff; // set all pins direction of PortD
  DDRC = 0xff; // set all pins direction of PortC
  DDRB = 0xff; // set all pins direction of PortB

  PORTD = 0x00; // set all pins output is low of PortD
  PORTC = 0x00; // set all pins output is low of PortC
  PORTB = 0x00; // set all pins output is low of PortB
#else // Colorduino goes here
  //set A0~A2 PD3~PD4 PD6~PD7 PB0~PB5 for digital output
  unsigned char lines[] = {A0, A1, A2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13};

  for (int i = 0; i < 13; i++) {
    pinMode(lines[i], OUTPUT);
    digitalWrite(lines[i], LOW);
  }
#endif
}


void ColorduinoObject::_TC2_Init() {
  // Arduino runs at 16 Mhz...
  // Timer Settings, for the Timer Control Register etc. , thank you internets. ATmega168 !
  // Timer2 (8bit) Settings:
  // prescaler (frequency divider) values:   CS22    CS21   CS20
  //                                           0       0      0    stopped
  //                                           0       0      1      /1
  //                                           0       1      0      /8
  //                                           0       1      1      /32
  //                                           1       0      0      /64
  //                                           1       0      1      /128
  //                                           1       1      0      /256
  //                                           1       1      1      /1024
  // TCNT2 increments every  = 16MHz / prescaler

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  TCCR2B = 0x00; // Disable timer while configuring
  TCCR2A = 0x00; // Use normal mode
  TIMSK2 = 0x01; // Timer2 overflow interrupt enable
  TCNT2  = 0xff; // Reset timer to count of 255
  TCCR2B = 0x05; // Prescaler = 128
#elif defined(__AVR_ATmega32U4__)
  // set prescaler to 128 -> TCNT2 freq = 125KHz
  //TCCR4B |= (1<<CS43);
  //TCCR4B &= ~((1<<CS42)|(1<<CS41)|(1<<CS40));
  TCCR4B |= ((1 << CS42) | (1 << CS41) | (1 << CS40));
  TCCR4C &= ~(1 << PWM4D);
  //TCCR4D &= ~((1<<WGM41)|(1<<WGM40));   // Use normal mode
  TCCR4A &= ~((1 << COM4A1) | (1 << COM4A0)); // Use normal mode
  TIMSK4 |= (1 << TOIE4);    //Timer4D Overflow Interrupt Enable
  TIFR4 |= (1 << TOV4);
  OCR4C = 0xff;
  TCNT4 = 0xff;
#else
  // set prescaler to 128 -> TCNT2 freq = 125KHz
  /* see \Arduino\hardware\tools\avr\avr\include\avr\iom328p.h
     TCCR2B = _SFR_MEM8(0xB1)
     CS20 = 0   //1<<CS22  = B0000001
     CS21 = 1   //1<<CS22  = B0000010
     CS22 = 2   //1<<CS22  = B0000100
     WGM20 = 0  //1<<WGM20 = B0000001
     WGM21 = 1  //1<<WGM21 = B0000010
     WGM22 = 3
     FOC2B = 6
     FOC2A = 7
  */
  /*
     see also ATmega328-328P_Datasheet.pdf - start from P189 - TC2
     [TCCR2B CS22-CS21-CS20 = 1-0-1] means prescaler = 128
     [TCCR2A WGM21-WGM20 = 0-0] means Waveform Generation Mode = Normal
     [ASSR AS2 = 1] means Timer/Counter2 is clocked from a crystal Oscillator connected to the Timer Oscillator 1 (TOSC1) pin
     [TIMSK2 TOIE2 = 1] means Timer/Counter2 Overflow interrupt is enabled
       The corresponding interrupt[ISP(TIMER2_OVF_vect)] is executed if an overflow in Timer/Counter2 occurs
  */
  TCCR2B |= ((1 << CS22) | (1 << CS20));
  TCCR2B &= ~((1 << CS21)); // set prescaler = 128

  TCCR2A &= ~((1 << WGM21) | (1 << WGM20)); // Use normal mode

  ASSR |= (1 << AS2);     // Use internal clock - external clock not used in Arduino

  TIMSK2 |= ((1 << TOIE2) | (0 << OCIE2B)); //Timer2 Overflow Interrupt Enable
  TCNT2 = 0xff;
#endif
  sei();
}


void ColorduinoObject::LED_Delay(unsigned char i)
{
  unsigned int y;
  y = i * 10;
  while (y--);
}

void ColorduinoObject::open_line(unsigned char x)
{
  switch (x)
  {
    case 0 : open_line0;
      break;
    case 1 : open_line1;
      break;
    case 2 : open_line2;
      break;
    case 3 : open_line3;
      break;
    case 4 : open_line4;
      break;
    case 5 : open_line5;
      break;
    case 6 : open_line6;
      break;
    case 7 : open_line7;
      break;
    default: close_all_lines;
      break;
  }
}

// compensate for relative intensity differences in R/G/B brightness
// array of 6-bit base values for RGB (0~63)
// wbval[0]=red
// wbval[1]=green
// wbval[2]=blue
void ColorduinoObject::SetWhiteBal(unsigned char wbval[3])
{
  LED_LAT_CLR;
  LED_SLB_CLR;
  for (unsigned char k = 0; k < ColorduinoScreenHeight; k++)
    for (unsigned char i = 3; i > 0 ; i--)
    {
      unsigned char temp = wbval[i - 1] << 2;
      for (unsigned char j = 0; j < ColorduinoBitsPerCorrection; j++)
      {
        if (temp & 0x80)
          LED_SDA_SET;
        else
          LED_SDA_CLR;

        temp = temp << 1;
        LED_SCL_CLR;
        LED_SCL_SET;
      }
    }
  LED_SLB_SET;
}



void ColorduinoObject::FlipPage(byte method = 0) {
  cli();
  // swap frame buffers
  PixelRGB *tmp = curDrawFrame;
  curDrawFrame = curWriteFrame;
  curWriteFrame = tmp;
  sei();
  if (method) {
    PixelRGB *p = curWriteFrame;
    PixelRGB *q = curDrawFrame;
    for (unsigned char y = 0; y < ColorduinoScreenWidth; y++) {
      for (unsigned char x = 0; x < ColorduinoScreenHeight; x++) {
        p->r = q->r;
        p->g = q->g;
        p->b = q->b;
        p++; q++;
      }
    }
  }
}


/********************************************************
  Name: ColorFill
  Function: Fill the frame with a color
  Parameter:R: the value of RED.   Range:RED 0~255
          G: the value of GREEN. Range:RED 0~255
          B: the value of BLUE.  Range:RED 0~255
********************************************************/
void ColorduinoObject::ColorFill(unsigned char R, unsigned char G, unsigned char B)
{
  PixelRGB *p = GetPixel(0, 0);
  for (unsigned char y = 0; y < ColorduinoScreenWidth; y++) {
    for (unsigned char x = 0; x < ColorduinoScreenHeight; x++) {
      p->r = R;
      p->g = G;
      p->b = B;
      p++;
    }
  }

  FlipPage();
}


void open_line(unsigned char x)
{
  switch (x)
  {
    case 0 : open_line0;
      break;
    case 1 : open_line1;
      break;
    case 2 : open_line2;
      break;
    case 3 : open_line3;
      break;
    case 4 : open_line4;
      break;
    case 5 : open_line5;
      break;
    case 6 : open_line6;
      break;
    case 7 : open_line7;
      break;
    default: close_all_lines;
      break;
  }
}



// global instance
ColorduinoObject Colorduino;

ISR(TIMER2_OVF_vect)          //Timer2  Service   Description: Timer/Counter2 Overflow
{
  // ISR fires every 256-TCNT2 ticks
  // so if TCNT2 = 100, ISR fires every 156 ticks
  // prescaler = 128 so ISR fires every 16MHz / 128 = 125KHz
  /*
     [TCCR2B CS22-CS21-CS20 = 101] means prescaler = 128
  */
  // 125KHz / 156 = 801.282Hz / 8 rows = 100.16Hz refresh rate
  // if TCNT2 = 61, ISR fires every 256 - 61 = 195 ticks
  // 125KHz / 195 = 641.026Hz / 8 rows = 80.128Hz refresh rate
  //  TCNT2 = 100;
  TCNT2 = 61;
  close_all_lines;
  Colorduino.run();
  Colorduino.open_line(Colorduino.line);
  if (++Colorduino.line > 7) Colorduino.line = 0;
}

/****************************************************
  the LED Hardware operate functions zone
****************************************************/

/***************************************************
  the LED datas operate functions zone
***************************************************/

void ColorduinoObject::run()
{
  LED_SLB_SET;
  LED_LAT_CLR;
  PixelRGB *pixel = GetDrawPixel(0, line);
  for (unsigned char x = 0; x < ColorduinoScreenWidth; x++)
  {
    unsigned char temp = pixel->b;
    unsigned char p;
    for (p = 0; p < ColorduinoBitsPerColor; p++) {
      if (temp & 0x80)
        LED_SDA_SET;
      else
        LED_SDA_CLR;
      temp <<= 1;
      LED_SCL_CLR;
      LED_SCL_SET;
    }
    temp = pixel->g;
    for (p = 0; p < ColorduinoBitsPerColor; p++) {
      if (temp & 0x80)
        LED_SDA_SET;
      else
        LED_SDA_CLR;
      temp <<= 1;
      LED_SCL_CLR;
      LED_SCL_SET;
    }
    temp = pixel->r;
    for (p = 0; p < ColorduinoBitsPerColor; p++) {
      if (temp & 0x80)
        LED_SDA_SET;
      else
        LED_SDA_CLR;
      temp <<= 1;
      LED_SCL_CLR;
      LED_SCL_SET;
    }
    pixel++;
  }
  LED_LAT_SET;
  LED_LAT_CLR;
}



    // get a pixel for writing in the offscreen framebuffer, return null if out of screen
    PixelRGB * ColorduinoObject::GetPixel(char x, char y) {
      if ( x < 0 || x >= ColorduinoScreenWidth ||
           y < 0 || y >= ColorduinoScreenHeight )
      {
        return NULL;
      }
      return curWriteFrame + (y * ColorduinoScreenWidth) + x;
    }

    // get a pixel from the active framebuffer, return null if out of screen
    PixelRGB * ColorduinoObject::GetDrawPixel(char x, char y) {
      if ( x < 0 || x >= ColorduinoScreenWidth ||
           y < 0 || y >= ColorduinoScreenHeight )
      {
        return NULL;
      }
      return curDrawFrame + (y * ColorduinoScreenWidth) + x;
    }

    // set a pixel in the offscreen frame buffer
    void ColorduinoObject::SetPixel(char x, char y, unsigned char r, unsigned char g, unsigned char b) {
      PixelRGB *p = GetPixel(x, y);
      if (p) {
        p->r = r;
        p->g = g;
        p->b = b;
      }
    }

    // set a pixel in the offscreen frame buffer
    void ColorduinoObject::SetPixel(char x, char y, const PixelRGB & color) {
      PixelRGB * p = GetPixel(x, y);
      if (p) {
        *p = color;
      }
    }

