#ifndef _COLORDUINO_I2C_
#define _COLORDUINO_I2C_

#include <EEPROM.h>
#include <Wire.h>                //wire library for I2C
#include <ColorduinoOptimized.h>          //colorduino library

#define START_OF_DATA 0x10       //data markers
#define END_OF_DATA 0x20         //data markers

class I2ColorduinoObject {
  public:
    void Init();
    void updateDisplay(byte, byte *);
    static void receiveEvent(int);
    static byte pixels[28];

};

extern I2ColorduinoObject I2Colorduino;

#endif // _COLORDUINO_I2C_
