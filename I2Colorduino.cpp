#include "I2Colorduino.h" // Engage Colorduino Library

// Class Member's Declaration != Variable's Definition
// Must Define Static Member Before Usage
byte I2ColorduinoObject::pixels[28];

void I2ColorduinoObject::Init() {

  // Init Colorduino Display Controlling Object
  Colorduino.Init();

  // Read Configuration Stored in EEPROM
  // byte[0] - I2C Address - mask 0x7F
  // byte[1~3] - White Balance RGB - mask 0x3F
  byte I2C_DEVICE_ADDRESS = EEPROM.read(0);
  byte wb_r = EEPROM.read(1) & 0x3F;
  byte wb_g = EEPROM.read(2) & 0x3F;
  byte wb_b = EEPROM.read(3) & 0x3F;

  // Set White Balance
  unsigned char whiteBalVal[3] = {wb_r, wb_g, wb_b};
  Colorduino.SetWhiteBal(whiteBalVal);

  // Join I2C Bus as Slave Device
  Wire.begin(I2C_DEVICE_ADDRESS);
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
    (q + lposi * 8 + x)->r = pixs[x * 1 * 3 + 0];
    (q + lposi * 8 + x)->g = pixs[x * 1 * 3 + 1];
    (q + lposi * 8 + x)->b = pixs[x * 1 * 3 + 2];
  }
}


// ISR Callback
// Handle Commands Quickly!
static void I2ColorduinoObject::receiveEvent(int num) {

  // Drop Wrong Message
  if (Wire.read() != START_OF_DATA) {
    //else handle error by reading remaining data until end of data marker (if available)
    while (Wire.available() > 0 && Wire.read() != END_OF_DATA) {}
    return;
  }

  // Read Message Length Byte
  byte len = Wire.read();
  byte action = 0;
  byte param = 0;

  // Read Action & Param Byte
  if (len--) action = Wire.read() & 0x0F;
  if (len--) param = Wire.read();

  // If Still Bytes Available, Read Them
  for (byte i = 0; i < len; i++) {
    pixels[i] = Wire.read();
  }

  // Read End Byte
  while (Wire.available() > 0 && Wire.read() != END_OF_DATA) {}

  // Act Command
  switch (action) {
    case 0x01:
      I2Colorduino.updateDisplay(param , pixels);
      break;
    case 0x02:
      Colorduino.FlipPage();
      break;
  }

}

// global instance
I2ColorduinoObject I2Colorduino;
