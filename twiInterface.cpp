#include "twiInterface.h"

#include <Wire.h> 

bool twiSendChar(char const data)
{
  return Wire.write(data) == 1;
}

bool twiCharAvailable( void )
{
  return Wire.available();
}

char twiReceiveChar( void )
{
  return Wire.read();
}

void twiInitialize()
{
  Wire.begin();

}

void twiSleep( void )
{
}