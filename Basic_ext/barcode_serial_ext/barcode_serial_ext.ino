#include <Wire.h>

#define LED 2

uint8_t *data;
uint8_t rcv = 0;
unsigned int count = 0;
bool barcode_received = false;
static  char msg[16] = {0};
volatile bool secondTime = false ;
bool blink = false;



void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);

  Serial.begin(9600);
  Wire.begin(5);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);


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
  if (count > 3) // if count is more than three means it is about handshake thing else it means button have been pressed
    Serial.write(0x06);
  else if (barcode_received)// when button pressed and pattern code received, it is time to send the code
  {
    Serial.write(0x02);
    Serial.write('U');

    Serial.print(msg);
    Serial.write(0x03);
    barcode_received = false;
  }
  else // if button pressed for second time :)
  {
    secondTime  = true ;
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



void requestEvent()
{
  if (secondTime == true)
  {
    Wire.write(0x5F);
    secondTime = false;
  }
  else
    //Wire.write("000000000002\n");
    Wire.write(0xff);
  if (blink == false)
  {
    blink = true;
    digitalWrite(LED, 1);
  }
  else
  {
    blink = false;
    digitalWrite(LED, 0);
  }

}
