
#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <queue.h>
#include <semphr.h>

#define     SUP_RFID    "79004C643D6Cxxx"
#define     MEC1_RFID   "4D0022EB8703xxx"
//#define     LED   13
# define LED 3

QueueHandle_t queue;



QueueHandle_t i2c_serial_ext_queue;
SemaphoreHandle_t xMutex_print ;
SemaphoreHandle_t xMutex_i2c ;
// define two tasks for Blink & AnalogRead
void TaskBlink( void *pvParameters );
void TaskAnalogRead( void *pvParameters );
void TaskI2C ( void *pvParameters );

// the setup function runs once when you press reset or power the board
void setup() {

  pinMode(LED, OUTPUT);

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

  xTaskCreate(
    TaskAnalogRead
    ,  (const portCHAR *) "AnalogRead"
    ,  128 // This stack size can be checked & adjusted by reading Highwater
    ,  NULL
    ,  1  // priority
    ,  NULL );

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
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
    digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
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
  char *str, c, c2, str2[16] = {0} , *str3 = "77777777777" ;
  //String str;
  //char buff [16];
  int i = 0;
  int j = 3000;

  char *i2c_msg = "";
  bool rfid_read = false;



  for (;;)
  {
    //if ( xSemaphoreTake( xMutex_print, portMAX_DELAY ) == pdTRUE )
    //{


    //      if ( xSemaphoreTake( xMutex_i2c, portMAX_DELAY ) == pdTRUE )
    //      {
    Wire.requestFrom(7, 13);    // request 6 bytes from slave device #8

    while (Wire.available()) { // slave may send less than requested
      c = Wire.read(); // receive a byte as character
      if (c != -1)  
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


    *(str2 + i) = 0 ;
    i = 0 ;

    //Serial.print(str2);

    //strcpy(str3, str2);




    if (rfid_read == true)
    {
      Serial.write(0x2);
      Serial.print(str2); //send RFID code
      Serial.write(0x3);
      Serial.println("");//if not println no pattern will be sent



      rfid_read  = false ;
    }

    strcpy(str2 , "") ;
    // xSemaphoreGive( xMutex_print );
    // }



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


    vTaskDelay(1500 / portTICK_PERIOD_MS ); // changed from 500 to 1500 to reduce I2C communication rate


  }
}
