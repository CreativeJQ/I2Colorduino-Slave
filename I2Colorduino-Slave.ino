#include "I2Colorduino.h" // Engage I2Colorduino Library

void setup()
{
  I2Colorduino.Init();
}

void loop()
{
  I2Colorduino.Dispatch();
}

