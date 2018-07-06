#ifndef _COLORDUINO_I2C_
#define _COLORDUINO_I2C_

#include <EEPROM.h>
#include <Wire.h>                //wire library for I2C
#include <ColorduinoOptimized.h>          //colorduino library
#include <avr/pgmspace.h>
#include "MatrixFont.h"
#include "JQueueBuffer.h" // Engage JQueue Buffer Library

#define START_OF_DATA 0x10       //data markers
#define END_OF_DATA 0x20         //data markers

#define CONFIG_STATUS_NORMAL 0xF0 // No need to fix config
#define CONFIG_STATUS_UNINITIALIZED1 0x00 // Need to fix config
#define CONFIG_STATUS_UNINITIALIZED2 0xFF // Need to fix config
#define CONFIG_STATUS_RECEIVED 0x01 // Need to fix config

/*
   I2Colorduino Protocol
*/
#define I2C_ACTION_NOTHING_TO_DO 0xFF
#define I2C_ACTION_RELOAD 0x00
#define I2C_ACTION_UPDATE_SINGLE_ROW 0x01
#define I2C_ACTION_UPDATE_REFRESH 0x02
#define I2C_ACTION_OFFSET 0x03
#define I2C_ACTION_CONFIG 0x04
#define I2C_ACTION_UPDATE_TEXT 0x05


class I2ColorduinoObject {
  private:
    static byte command[32];
    static byte pixels[28];
    static byte status_flag;
    static byte len;
    static byte action;
    static byte param;
    static byte offset_x;
    static byte offset_y;
    static byte textt;
    //static byte textLayer[ColorduinoScreenWidth * ColorduinoScreenHeight * 4];
    void InitMatrixFont();

  public:
    void Init();
    void Dispatch();
    void updateDisplay(byte, byte *);
    void updateDisplay2(byte, byte *);
    static void receiveEvent(int);
    void Shutdown();
    void Reload();
    void Config(byte, byte, byte, byte, byte);
    void FixConfig(byte);
    byte alphaBlend(byte, byte, byte, byte);
    void alphaBlend_Pixel_RGBA(PixelRGB * , byte, byte, byte, byte);
    void alphaBlend_Pixel_RGBA2(PixelRGB * , byte, byte, byte, byte);
    void alphaBlend_Pixel_XYRGBA(PixelRGB * , byte, byte, byte, byte, byte, byte);
    void alphaBlend_Pixel_XYRGBA2(PixelRGB * , byte, byte, byte, byte, byte, byte);
    void Offset(byte, byte);
    void updateText(byte, byte, byte, PixelRGB);
    void updateMatrixFontText(byte, byte, byte, PixelRGB, byte matrixIndex = 0);
    void updateMatrixFontTextOffset(byte, byte, byte, byte, byte, byte, byte,  PixelRGB);
    void CombineTextLayer();
    void updateMatrixFontDot(byte, byte, PixelRGB, byte);
    void Refresh();
};

extern I2ColorduinoObject I2Colorduino;

#endif // _COLORDUINO_I2C_
