# I2Colorduino-Slave
## How to use
- Import libraries to your Arduino IDE
- Copy I2Colorduino-Slave.ino, I2Colorduino.h, I2Colorduino.cpp to folder I2Colorduino-Slave
- Open I2Colorduino-Slave.ino with Arduino
- Connect your Colorduino like device with usb-ttl cable
- Upload sketch
- Pin up I2C master to drive it
## Basic configuration
### EEPROM
- byte 0 : I2C Slave Address, mask 0x7F
- byte 1~3 : White Balance Config, R - G - B, mask 0x3F
### I2C
- Master GND - Slave GND
- Master SCL - Slave SCL
- Master SDA - Slave SDA
- Master 5V \ Slave 5V (can be supplied separately)
