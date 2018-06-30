#include "I2Colorduino.h" // Engage I2Colorduino Library

// Class Member's Declaration != Variable's Definition
// Must Define Static Member Before Usage
byte I2ColorduinoObject::command[32];
byte I2ColorduinoObject::pixels[28];
byte I2ColorduinoObject::status_flag = 0x00;
byte I2ColorduinoObject::len = 0;
byte I2ColorduinoObject::action = 0;
byte I2ColorduinoObject::param = 0;
byte I2ColorduinoObject::offset_x = 0;
byte I2ColorduinoObject::offset_y = 0;
byte I2ColorduinoObject::textt = 0;

void I2ColorduinoObject::Init() {

  // Init Colorduino Display Controlling Object
  Colorduino.Init();

  byte fixConfig_flag = EEPROM.read(4);
  if (fixConfig_flag != CONFIG_STATUS_NORMAL) FixConfig(fixConfig_flag);

  // Read Configuration Stored in EEPROM
  // byte[0] - I2C Address - mask 0x7F
  // byte[1~3] - White Balance RGB - mask 0x3F
  byte addr = EEPROM.read(0);
  byte wb_r = EEPROM.read(1) & 0x3F;
  byte wb_g = EEPROM.read(2) & 0x3F;
  byte wb_b = EEPROM.read(3) & 0x3F;

  // Set White Balance
  unsigned char whiteBalVal[3] = {wb_r, wb_g, wb_b};
  Colorduino.SetWhiteBal(whiteBalVal);

  // Join I2C Bus as Slave Device
  Wire.begin(addr);

  // Tweak: Modify TWAR Register to Receive General Call
  // Wire.h Lib Ignore TWGCE Bit for Convenience
  // But We Need to Fix It for Sync-Refresh Command
  // See Atmega328p.pdf - 26.9.3. TWI (Slave) Address Register
  TWAR = TWAR | (0x01 << TWGCE);

  // Command Dealing Callback
  Wire.onReceive(I2Colorduino.receiveEvent);   // define the receive function for receiving data from master. After end_transmission

}


void I2ColorduinoObject::updateDisplay(byte params, byte * pixs) {
  /* 8bit params H to L
      1bit Line Mode : 1-Horiz 0-Verti
      3bit Line Position : 0 ~ 7
      1bit Reserve Flag : 1-ReserveLastImage 0-ClearLastImage
      1bit Colour Mode : 1-RGBA 0-RGB
      2bit Colour Mask : 0,0-1bit 0,1-2bit 1,0-4bit 1,1-8bit ([b1,b2]-[Colour Bit Depth:1/2/4/8])
  */
  byte lmode = (params & 0x80) >> 7;
  byte lposi = (params & 0x7F) >> 4;
  byte resrv = (params & 0x08) >> 3;
  byte cmode = (params & 0x04) >> 2;
  byte cshft = (byte)(params & 0x03);
  byte cmask = 0x01 << cshft;

  PixelRGB *q = Colorduino.GetPixel(0, 0);

  for (byte x = 0; x < 8; x++) {
    alphaBlend_Pixel_RGBA((PixelRGB * )(q + lposi * 8 + x), pixs[x * 1 * 3 + 0], pixs[x * 1 * 3 + 1], pixs[x * 1 * 3 + 2], 0);
  }
}


void I2ColorduinoObject::updateDisplay2(byte params, byte * pixs) {
  /* 8bit params H to L
      1bit Line Mode : 1-Horiz 0-Verti
      3bit Line Position : 0 ~ 7
      1bit Reserve Flag : 1-ReserveLastImage 0-ClearLastImage
      1bit Colour Mode : 1-RGBA 0-RGB
      2bit Colour Mask : 0,0-1bit 0,1-2bit 1,0-4bit 1,1-8bit ([b1,b2]-[Colour Bit Depth:1/2/4/8])
  */
  byte lmode = (params & 0x80) >> 7;
  byte lposi = (params & 0x7F) >> 4;
  byte resrv = (params & 0x08) >> 3;
  byte cmode = (params & 0x04) >> 2;
  byte cshft = (byte)(params & 0x03);
  byte cmask = 0x01 << cshft;

  PixelRGB *q = Colorduino.GetPixel(0, 0);
  PixelRGB *q2 = Colorduino.GetDrawPixel(0, 0);

  for (byte x = 0; x < 8; x++) {
    byte dot_r = (q2 + lposi * 8 + x)->r;
    byte dot_g = (q2 + lposi * 8 + x)->g;
    byte dot_b = (q2 + lposi * 8 + x)->b;
    byte pix_r = pixs[x * 1 * 3 + 0];
    byte pix_g = pixs[x * 1 * 3 + 1];
    byte pix_b = pixs[x * 1 * 3 + 2];

    (q + lposi * 8 + x)->r = 255 - dot_r > pix_r ? dot_r + pix_r : 255;
    (q + lposi * 8 + x)->g = 255 - dot_g > pix_g ? dot_g + pix_g : 255;
    (q + lposi * 8 + x)->b = 255 - dot_b > pix_b ? dot_b + pix_b : 255;
  }
}


// Dispatcher
void I2ColorduinoObject::Dispatch() {
  if (JQueue.getLength() == 0){ return;}
  byte tmpCommand[32];
  JQueue.pop(tmpCommand);

  // Read Message Length Byte
  len = tmpCommand[0];

  // Read Action & Param Byte
  action = tmpCommand[1] & 0x0F;
  param = tmpCommand[2];

  // If Still Bytes Available, Read Them
  for (byte i = 0; i < len; i++) {
    pixels[i] = tmpCommand[i + 3];
    //Serial.print(pixels[i]);
    //Serial.print(" , ");
  }

  PixelRGB cccc;
  cccc.r = 255;
  cccc.g = 255;
  cccc.b = 255;
  byte * tttt;
  //byte a[1] = {"0a"};
  byte a = 48;

  
  // Act Command
  switch (action) {
    case I2C_ACTION_RELOAD:
      I2Colorduino.Reload();
      break;
    case I2C_ACTION_UPDATE_SINGLE_ROW:
      I2Colorduino.updateDisplay(param , pixels);
      break;
    case I2C_ACTION_UPDATE_REFRESH:
      //I2Colorduino.updateMatrixFontText(1 & 0x7F, 5, 3, cccc);
      //I2Colorduino.updateMatrixFontText(textt & 0x7F, 0, 0, cccc);
      I2Colorduino.updateMatrixFontText(textt++ & 0x7F, 4, 0, cccc);
      I2Colorduino.Refresh();
      break;
    case I2C_ACTION_OFFSET:
      I2Colorduino.Offset(param, pixels[0]);
      break;
    case I2C_ACTION_CONFIG:
      I2Colorduino.Config(param, pixels[0], pixels[1], pixels[2], pixels[3]);
      break;
  }
}


// ISR Callback
// Handle Commands Quickly!
void I2ColorduinoObject::receiveEvent(int num) {

  byte tmpCommand[32];
  
  // Drop Wrong Message
  if (Wire.read() != START_OF_DATA) {
    //else handle error by reading remaining data until end of data marker (if available)
    while (Wire.available() > 0 && Wire.read() != END_OF_DATA) {}
    return;
  }

  for (byte i = 0; i < 32; i++) {
    tmpCommand[i] = Wire.read();
  }

  //Serial.print("\n");
  JQueue.push(tmpCommand);

  while (Wire.available() > 0 && Wire.read() != END_OF_DATA) {}
}


void I2ColorduinoObject::Shutdown() {
  // Quit I2C Bus
  Wire.end();
}


void I2ColorduinoObject::Reload() {
  Shutdown();
  Init();
}


void I2ColorduinoObject::Refresh() {
  //  CombineTextLayer();
  Colorduino.FlipPage();
}


void I2ColorduinoObject::Config(byte addr, byte wb_r, byte wb_g, byte wb_b, byte needReload_flag) {
  // use EEPROM.update() instead of write() to save cycles
  EEPROM.update(5, addr);
  EEPROM.update(6, wb_r);
  EEPROM.update(7, wb_g);
  EEPROM.update(8, wb_b);
  EEPROM.update(4, CONFIG_STATUS_RECEIVED);
  EEPROM.update(0, addr);
  EEPROM.update(1, wb_r);
  EEPROM.update(2, wb_g);
  EEPROM.update(3, wb_b);
  EEPROM.update(4, CONFIG_STATUS_NORMAL);

  // Reload settings
  if (needReload_flag) Reload();
}


void I2ColorduinoObject::FixConfig(byte flag) {
  // use EEPROM.update() instead of write() to save cycles
  switch (flag) {
    case CONFIG_STATUS_RECEIVED: {
        byte addr = EEPROM.read(5);
        byte wb_r = EEPROM.read(6) & 0x3F;
        byte wb_g = EEPROM.read(7) & 0x3F;
        byte wb_b = EEPROM.read(8) & 0x3F;
        EEPROM.update(0, addr);
        EEPROM.update(1, wb_r);
        EEPROM.update(2, wb_g);
        EEPROM.update(3, wb_b);
        EEPROM.update(4, CONFIG_STATUS_NORMAL);
      }
      // Add { } to avoid error: [crosses initialization]
      break;
    case CONFIG_STATUS_UNINITIALIZED1:
    case CONFIG_STATUS_UNINITIALIZED2:
    default:
      EEPROM.update(0, 2);
      EEPROM.update(1, 63);
      EEPROM.update(2, 63);
      EEPROM.update(3, 63);
      break;
  }
}


byte I2ColorduinoObject::alphaBlend(byte color1, byte alpha1, byte color2, byte alpha2) {
  byte abResult = (byte)(((long)color1 * (long)alpha1 + (long)color2 * (long)alpha2) / 255);
  return abResult;
}


void I2ColorduinoObject::alphaBlend_Pixel_RGBA(PixelRGB * pixel_p, byte r, byte g, byte b, byte alpha) {
  pixel_p -> r = alphaBlend(pixel_p -> r, alpha, r, 255 - alpha);
  pixel_p -> g = alphaBlend(pixel_p -> g, alpha, g, 255 - alpha);
  pixel_p -> b = alphaBlend(pixel_p -> b, alpha, b, 255 - alpha);
}


void I2ColorduinoObject::alphaBlend_Pixel_RGBA2(PixelRGB * pixel_p, byte r, byte g, byte b, byte alpha) {
  pixel_p -> r = alphaBlend(0, alpha, r, 255 - alpha);
  pixel_p -> g = alphaBlend(0, alpha, g, 255 - alpha);
  pixel_p -> b = alphaBlend(0, alpha, b, 255 - alpha);
}


void I2ColorduinoObject::alphaBlend_Pixel_XYRGBA(PixelRGB * pixel_p, byte x, byte y, byte r, byte g, byte b, byte alpha) {
  byte _x = x - offset_x;
  byte _y = y - offset_y;
  if (_x < 0 || _y < 0 || _x >= ColorduinoScreenWidth || _y >= ColorduinoScreenHeight)
    return;
  (pixel_p + _x + +y * ColorduinoScreenWidth) -> r = alphaBlend(pixel_p -> r, alpha, r, 255 - alpha);
  (pixel_p + _x + +y * ColorduinoScreenWidth) -> g = alphaBlend(pixel_p -> g, alpha, g, 255 - alpha);
  (pixel_p + _x + +y * ColorduinoScreenWidth) -> b = alphaBlend(pixel_p -> b, alpha, b, 255 - alpha);
}


void I2ColorduinoObject::alphaBlend_Pixel_XYRGBA2(PixelRGB * pixel_p, byte x, byte y, byte r, byte g, byte b, byte alpha) {
  pixel_p -> r = alphaBlend(0, alpha, r, 255 - alpha);
  pixel_p -> g = alphaBlend(0, alpha, g, 255 - alpha);
  pixel_p -> b = alphaBlend(0, alpha, b, 255 - alpha);
}


void I2ColorduinoObject::Offset(byte x, byte y) {
  offset_x = x;
  offset_y = y;
}


void I2ColorduinoObject::updateText(byte text, byte x, byte y, PixelRGB color) {

  unsigned char i, j = 0, Page_Write, temp;
  unsigned char Char;
  unsigned char chrtemp[24] = {0};

  PixelRGB *q = Colorduino.GetPixel(0, 0);

  Char = text - 32;

  for (i = 0; i < 8; i++)
  {
//    chrtemp[j] = pgm_read_byte(&(font8_8[Char][i]));
    j++;
  }
  for (i = 0; i < 8; i++)
  {
    temp = chrtemp[i];
    for (j = 0; j < 8; j++)
    {
      if (temp & 0x80)
      {
        updateMatrixFontDot(i, j, color, 0);
      }
      else
      {
        updateMatrixFontDot(i, j, color, 255);
      }
      temp = temp << 1;
    }
  }
}



void I2ColorduinoObject::updateMatrixFontText(byte text, byte x, byte y, PixelRGB color) {

  unsigned char i, j = 0;
  unsigned char chrtmp, chrtmp_H, chrtmp_L;

  PixelRGB *q = Colorduino.GetPixel(0, 0);

  chrtmp_H = pgm_read_byte(&(MatrixFrontA35[text][0]));
  chrtmp_L = pgm_read_byte(&(MatrixFrontA35[text][1]));

  for (i = 0 ; i < 7; i++) {
    chrtmp_H = chrtmp_H << 1;
    if (chrtmp_H & 0x80) {
      //updateMatrixFontDot(j % 3 + x, j / 3 + y, color, 0);
      alphaBlend_Pixel_XYRGBA(q, j % 3 + x, j / 3 + y, color.r, color.g, color.b, 0);
    }
    else {
      //updateMatrixFontDot(j % 3 + x, j / 3 + y, color, 255);
    }
    j++;
  }
  for (i = 0 ; i < 8; i++) {
    if (chrtmp_L & 0x80) {
      //updateMatrixFontDot(j % 3 + x, j / 3 + y, color, 0);
      alphaBlend_Pixel_XYRGBA(q, j % 3 + x, j / 3 + y, color.r, color.g, color.b, 0);
    }
    else {
      //updateMatrixFontDot(j % 3 + x, j / 3 + y, color, 255);
    }
    chrtmp_L = chrtmp_L <<  1;
    j++;
  }

}
void I2ColorduinoObject::updateMatrixFontDot(byte x, byte y, PixelRGB color, byte alpha) {

  /*
    textLayer[(x + y * ColorduinoScreenWidth) * 4 + 0] = color.r;
    textLayer[(x + y * ColorduinoScreenWidth) * 4 + 1] = color.g;
    textLayer[(x + y * ColorduinoScreenWidth) * 4 + 2] = color.b;
    textLayer[(x + y * ColorduinoScreenWidth) * 4 + 3] = alpha;
  */
}

void I2ColorduinoObject::updateMatrixFontTextOffset(byte text, byte x, byte y, byte _up, byte _right, byte _down, byte _left, PixelRGB color) {

  unsigned char i, j = 0;
  unsigned char chrtmp, chrtmp_H, chrtmp_L;

  PixelRGB *q = Colorduino.GetPixel(0, 0);

  chrtmp_H = pgm_read_byte(&(MatrixFrontA35[text][0]));
  chrtmp_L = pgm_read_byte(&(MatrixFrontA35[text][1]));

  for (i = 0 ; i < 7; i++) {
    chrtmp_H = chrtmp_H << 1;
    if (chrtmp_H & 0x80) {
      //alphaBlend_Pixel_XYRGBA(q, j % 3 + x, j / 3 + y, color.r, color.g, color.b, 0);
    }
    j++;
  }
  for (i = 0 ; i < 8; i++) {
    if (chrtmp_L & 0x80) {
      //alphaBlend_Pixel_XYRGBA(q, j % 3 + x, j / 3 + y, color.r, color.g, color.b, 0);
    }
    chrtmp_L = chrtmp_L <<  1;
    j++;
  }

}

void I2ColorduinoObject::CombineTextLayer() {

  PixelRGB *q = Colorduino.GetPixel(0, 0);

  for (byte i = 0; i < ColorduinoScreenWidth; i++)
    for (byte j = 0; j < ColorduinoScreenHeight; j++);
  //      alphaBlend_Pixel_XYRGBA(q, i, j, textLayer[(i + j * ColorduinoScreenWidth) * 4 + 0], textLayer[(i + j * ColorduinoScreenWidth) * 4 + 1], textLayer[(i + j * ColorduinoScreenWidth) * 4 + 2], textLayer[(i + j * ColorduinoScreenWidth) * 4 + 3]);

}


// global instance
I2ColorduinoObject I2Colorduino;
