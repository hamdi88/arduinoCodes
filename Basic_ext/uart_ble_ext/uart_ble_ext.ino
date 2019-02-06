#include <Wire.h>

#define LED 2
uint8_t *data;
uint8_t rcv_char = 0;
unsigned int count = 0;
bool rfid_received = false;
bool pattern_received = false ;
int i = 0;

static  char msg[16] = {0};
char pattern_code [6] = {0};
bool blink = false;


void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);

  Serial.begin(115200);
  Wire.begin(3);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);


  delay(1000);
  Serial.println("UART BLE Extension");


}

void loop() {
  // put your main code here, to run repeatedly:


  if (rfid_received)
  {
    Serial.write(0x02);
    Serial.write(msg, 10);
    Serial.write(0x03);
    rfid_received = false;
    strcpy(msg, "");
  }

  count = 0;
  //Serial.println("-------");

  if (Serial.available())
  {
    rcv_char =  Serial.read();
    if (rcv_char != 0x0d)
    {
      //Serial.print(rcv_char);
      if (rcv_char  == 0x02)
      {
        i = 0;

      }
      else
      {
        pattern_code[i++] = rcv_char;
      }
    }
    else
    {
      pattern_code [i] = 0;
     // Serial.print (rfid_code);
      pattern_received = true;
    }
  }

}


//*******************************************************//

void receiveEvent(int howMany)
{
  static int i = 0 ;
  digitalWrite(LED , 0);

  while (Wire.available())
  {
    char c = Wire.read();

    //  Serial.println(c);
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


      rfid_received = true;
      i = 0;
      digitalWrite(LED , 1);
    }

  }
}

void requestEvent()
{
  if (pattern_received == true)
  {
    Wire.write(pattern_code, 6);
    pattern_received = false;
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
