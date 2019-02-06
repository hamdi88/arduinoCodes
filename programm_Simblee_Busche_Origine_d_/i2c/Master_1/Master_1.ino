
//i2c Master Code(UNO)
#include <Wire.h>
char c  , str2[32];
int i ;

void setup()
{
  Serial.begin(9600);

  Wire.begin();
}

void loop()
{
  Wire.requestFrom(7, 13);    // request 6 bytes from slave device #2

  while(Wire.available())    // slave may send less than requested
  {
    char c = Wire.read();    // receive a byte as character
    Serial.print(c);         // print the character
  }

  delay(500);
}
