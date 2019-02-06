#include <Wire.h>


uint8_t *data;
uint8_t rcv = 0;
unsigned int count = 0;
bool barcode_received = false;
static  char msg[16] = {0};


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin(5);
  Wire.onReceive(receiveEvent);

}

void loop() {
  // put your main code here, to run repeatedly:

  while (rcv != 0x0d)
  {
    if (Serial.available() > 0)
    {
      rcv = Serial.read();

      //*data++ = rcv;
     // Serial.println(rcv, HEX);
      count ++;
    }
  }
  //Serial.println("-------");
  rcv = 0;
  //*data++ = '\0';
  if (count > 3)
    Serial.write(0x06);
  else if (barcode_received)
  {
    Serial.write(0x02);
    Serial.print(msg);
    Serial.write(0x03);
    barcode_received = false;
  }

  count = 0;
  //Serial.println("-------");
}


//*******************************************************//

void receiveEvent(int howMany)
{
  static int i = 0 ;

  while (Wire.available())
  {
    char c = Wire.read();
    //Serial.println(c);
    if (c != ';')
    {
      msg[i++] = c;
    }
    else
    {
      msg[i] = 0;
      //      Serial.write(0x02);
      //      Serial.print(msg);
      //      Serial.write(0x03);


      barcode_received = true;
      i = 0;
    }

  }
}
