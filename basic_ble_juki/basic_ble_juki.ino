
#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <SPI.h>

#include <queue.h>
#include <semphr.h>
#include "PinChangeInterrupt.h"

#define     SUP_RFID    "79004C643D6Cxxx"
#define     MEC1_RFID   "4D0022EB8703xxx"
//#define     LED   13
#define LED 29
#define PlusPin 22 // //changed to meet plc basic hardware 
#define StitchPin 23 // //changed to meet plc basic hardware 
#define KlammerPin 24 // //changed to meet plc basic hardware 
QueueHandle_t queue;


volatile bool plusOne_triggered = false ;
volatile bool stitch_triggered = false ;
volatile bool klammer_triggered = false ;

volatile unsigned long plusOne_counter  = 0 ;
volatile unsigned long stitch_counter  = 0 ;


QueueHandle_t i2c_serial_ext_queue;
SemaphoreHandle_t xMutex_print ;
SemaphoreHandle_t xMutex_i2c ;
// define two tasks for Blink & AnalogRead
void TaskBlink( void *pvParameters );
void TaskAnalogRead( void *pvParameters );
void TaskI2C ( void *pvParameters );

// the setup function runs once when you press reset or power the board
void setup() {

  SPI.end();
  pinMode(LED, OUTPUT);
  pinMode(KlammerPin, INPUT);

  attachPCINT(digitalPinToPCINT(PlusPin), plusOne, FALLING);
  attachPCINT(digitalPinToPCINT(StitchPin), stitchCounting, FALLING);
  attachPCINT(digitalPinToPCINT(KlammerPin), klammerUp, FALLING);


  Serial.begin(9600);
  Wire.begin();
  // Now set up two tasks to run independently.
  xTaskCreate(
    TaskBlink
    ,  (const portCHAR *)"Blink"   // A name just for humans
    ,  128  // Stack size
    ,  NULL
    ,  2  // priority
    ,  NULL );

  //  xTaskCreate(
  //    TaskAnalogRead
  //    ,  (const portCHAR *) "AnalogRead"
  //    ,  128 // This stack size can be checked & adjusted by reading Highwater
  //    ,  NULL
  //    ,  1  // priority
  //    ,  NULL );

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.


  xTaskCreate(
    TaskI2C
    ,  (const portCHAR *) "I2C_UART"
    ,  256 // This stack size can be checked & adjusted by reading Highwater
    ,  NULL
    ,  2  // priority
    ,  NULL );

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
};

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBlink(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);
  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)

    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
    digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW

    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second


    // stitch_triggered  = false ;
    Serial.print("stitchs: ");
    Serial.println(stitch_counter) ;

  }
}
//***************************************************************
//***************************************************************
void TaskAnalogRead(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  // initialize serial communication at 9600 bits per second:

  for (;;)
  {
    // read the input on analog pin 0:
    int sensorValue = analogRead(A0);
    // print out the value you read:
    // Serial.println(sensorValue);
    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}



//***************************************************************
//***************************************************************


void TaskI2C(void *pvParameters)
{



  char element[16] ;
  char *str, c, c2, str2[16] = {0}  , pattern_msg[7] = {0};
  //String str;
  //char buff [16];
  int i = 0;
  int j = 3000;

  char *i2c_msg = "";
  bool rfid_read = false, pattern_read = false;

  int ret ;



  for (;;)
  {
    //if ( xSemaphoreTake( xMutex_print, portMAX_DELAY ) == pdTRUE )
    //{


    //      if ( xSemaphoreTake( xMutex_i2c, portMAX_DELAY ) == pdTRUE )
    //      {
    Wire.requestFrom(7, 13);    // request rfid form rfid_extension board with I2C address 0x07

    while (Wire.available()) { // slave may send less than requested
      c = Wire.read(); // receive a byte as character
      if (c != -1)// Character always received when no rfid have benn read(-1 == 0xFF)
      {
        // Serial.print(c); // print the character
        digitalWrite(LED , 1);
        *(str2 + i) = c;
        i++;

        rfid_read = true;
      }
      else
      {
        digitalWrite(LED , 0);

      }
    }
    // xSemaphoreGive( xMutex_i2c );
    //}


    *(str2 + i++) = ';' ;//*(str2 + i) = 0 ; the ';' Character acts like a delimiter to notify the end of data in I2C communication
    *(str2 + i++) = '\0';
    i = 0 ;



    if (rfid_read == true)
    {
      Serial.write(0x2);
      Serial.print(str2); //send RFID code
      Serial.write(0x3);
      Serial.println("");//if not println no pattern will be sent*/ // this portion of code used to send rfid to MTbox or else via UART now we need to send it to UART-BLE extension via I2C
      delay(500);
      Wire.beginTransmission(3); //begin transmission to UART-BLE extension
      Wire.write(str2);
      ret = Wire.endTransmission();

      Serial.print("rfid sent to uart-BLE with: :");
      Serial.println(ret);
      rfid_read  = false ;
    }

    strcpy(str2 , "") ;
    // xSemaphoreGive( xMutex_print );
    // }


    Wire.requestFrom(3, 6);    // request pattern form ble_extension board with I2C address 0x03

    while (Wire.available()) { // slave may send less than requested
      c = Wire.read(); // receive a byte as character
      if (c != -1)// Character always received when no rfid have benn read(-1 == 0xFF)
      {
        // Serial.print(c); // print the character
        digitalWrite(LED , 1);
        *(pattern_msg + i) = c;
        i++;

        pattern_read = true;
      }
      else
      {
        digitalWrite(LED , 0);

      }
    }
    i = 0;

    if (pattern_read == true)
    {
      pattern_read = false ;
      pattern_msg[3]  = ';' ;
      pattern_msg[4] = 0 ;
      Wire.beginTransmission(5);


      Wire.write(pattern_msg);
      Wire.endTransmission();
      Serial.print("Pattern code received: ");
      Serial.println(pattern_msg);

    }


    while (Serial.available())
    {
      c2 = Serial.read();
      if (c2 != '\n')
      {
        i2c_msg[i++] = c2;
      }
      else
      {
        i2c_msg[i++] = ';';
        i2c_msg[i] = '\0';
        //i2c_str += 0;
        //  xQueueSend(i2c_serial_ext_queue, &i2c_msg, portMAX_DELAY);

        Wire.beginTransmission(5);


        Wire.write(i2c_msg);
        Wire.endTransmission();
        i = 0;

      }


    }

    if (plusOne_triggered == true)
    {
      plusOne_triggered = false ;
      Wire.beginTransmission(3); //begin transmission to UART-BLE extension
      Wire.write(0x5F);
      Wire.write(';');
      Wire.endTransmission();
      Serial.print("PlusOne: ");
      Serial.println(plusOne_counter);
    }

    // It turns out that the buton return nothing when pressed for 2nd time , so next section is commented

    //    Wire.requestFrom(5, 1);    // request data form serial_extension board with I2C address 0x05 to see if button pressed
    //
    //    while (Wire.available()) { // slave may send less than requested
    //      c = Wire.read(); // receive a byte as character
    //      if (c != -1)// Character always received when no rfid have benn read(-1 == 0xFF)
    //      {
    //        Wire.beginTransmission(3); //begin transmission to UART-BLE extension
    //        Wire.write(0x5F);
    //        Wire.write(';');
    //        Wire.endTransmission();
    //      }
    //    }


    // instead we used the pin input to see if button pressed in other word when klammer is lifted up

    if (klammer_triggered == true)
    {
      klammer_triggered =  false ;
      Wire.beginTransmission(3); //begin transmission to UART-BLE extension
      Wire.write(0x5F);
      Wire.write(';');
      Wire.endTransmission();
      Serial.println("Klammer is Up");
    }
    vTaskDelay(250 / portTICK_PERIOD_MS ); // changed from 500 to 1500 to reduce I2C communication rate


  }
}

void plusOne()
{
  plusOne_triggered = true ;
  plusOne_counter ++ ;
}

void stitchCounting()
{
  stitch_counter ++ ;
  stitch_triggered = true ;
}

void klammerUp()
{
  klammer_triggered = true  ;
}
