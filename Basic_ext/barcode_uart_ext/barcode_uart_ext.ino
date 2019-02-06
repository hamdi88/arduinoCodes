#include <Wire.h>

# define LED 2


char rcv_char;
char rfid_code [13] = {0};
int i = 0;
bool is_rfid_read = false ;
bool blink = false;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);
  
  Serial.println ("uart extension");
  
  Wire.begin(7);

  delay(1000);
  Wire.onRequest(requestEvent);

  pinMode(LED, OUTPUT);
  
  


}

void loop() {
  // put your main code here, to run repeatedly:







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
        rfid_code[i++] = rcv_char;
      }
    }
    else
    {
      rfid_code [i] = 0;
      Serial.print (rfid_code);
      is_rfid_read = true;
    }
  }
}

void requestEvent()
{
  if (is_rfid_read == true)
  {
    Wire.write(rfid_code);
    is_rfid_read = false;
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
