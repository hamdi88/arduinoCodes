//i2c Slave Code(Leonardo)
#include <Wire.h>

#define   LED   3

static  char msg[16] = {0};
enum STATE {ON, OFF, BLINK} state;

void setup()
{
  Serial.begin(9600);
  Wire.begin(5);
  Wire.onReceive(receiveEvent);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  state = OFF;

   //Serial.println("Serial Extension from Marabout Technology");
   //Serial.println("-----------------------------------------");
}

void loop()
{

  switch (state)
  {
    case OFF :
      digitalWrite(LED, LOW);

      if (strcmp(msg, "ON\r") == 0)
      {
        state = ON;
      }
      else if (strcmp(msg, "BLINK\r") == 0)
        state = BLINK;
      break;

    case ON :
      digitalWrite(LED, HIGH);
      if (strcmp(msg, "OFF\r") == 0)
      {
        state = OFF;
      }
      else if (strcmp(msg, "BLINK\r") == 0)
        state = BLINK;

      break;

    case BLINK:

      if (digitalRead(LED) == HIGH)
      {
        digitalWrite(LED, LOW);
      }
      else
      {
        digitalWrite(LED, HIGH);

      }
      delay(350);
      
      if (strcmp(msg, "ON\r") == 0)
      {
        state = ON;
      }
      else if (strcmp(msg, "OFF\r") == 0)
        state = OFF;
      break;

  }

  /*if (strcmp(msg, "ON\r") == 0)
  {
    digitalWrite(LED, HIGH);
  }
  else if (strcmp(msg, "OFF\r") == 0)
  {
    digitalWrite(LED, LOW);
  }
  else if (strcmp(msg, "BLINK\r") == 0)
  {
    if (digitalRead(LED) == HIGH)
    {
      digitalWrite(LED, LOW);
    }
    else
    {
      digitalWrite(LED, HIGH);

    }
    delay(350);
  }*/
}

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
      Serial.write(0x02);
      Serial.print(msg);
      Serial.write(0x03);


      i = 0;
    }
   
  }
}
