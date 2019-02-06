#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <Wire.h>

#define     SUP_RFID    "79004C643D6Cxxx"
#define     MEC1_RFID   "4D0022EB8703xxx"
#define     LED   32

QueueHandle_t queue;
QueueHandle_t i2c_serial_ext_queue;
SemaphoreHandle_t xMutex_print ;
SemaphoreHandle_t xMutex_i2c ;

void TaskBlink( void *pvParameters );

void TaskPrint( void *pvParameters );
void TaskI2C( void *pvParameters );
void TaskI2C_SerialExt( void *pvParameters );


static TaskHandle_t xTaskBlink = NULL, xTaskAnanlog = NULL, xTaskPrint = NULL , xTaskI2C, xTaskI2C_SerialExt = NULL;




// the setup function runs once when you press reset or power the board
//***************************************************//

void setup() {


  Serial.begin(9600);
  Wire.begin();

  queue = xQueueCreate( 10, sizeof( int ) );
  i2c_serial_ext_queue = xQueueCreate(10, sizeof(char*));

  xMutex_print = xSemaphoreCreateMutex();
  xMutex_i2c = xSemaphoreCreateMutex();

  // Now set up two tasks to run independently.
  xTaskCreate(
    TaskBlink
    ,  (const portCHAR *)"Blink"   // A name just for humans
    ,  128  // Stack size
    ,  NULL
    ,  2  // priority
    ,  &xTaskBlink );


  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
  xTaskCreate(
    TaskPrint
    ,  (const portCHAR *) "Print"
    ,  128 // This stack size can be checked & adjusted by reading Highwater
    ,  NULL
    ,  2  // priority
    ,  &xTaskPrint );

  xTaskCreate(
    TaskI2C
    ,  (const portCHAR *) "I2c"
    ,  256 // This stack size can be checked & adjusted by reading Highwater
    ,  NULL
    ,  3  // priority
    ,  &xTaskI2C );



  xTaskCreate(
    TaskI2C_SerialExt
    ,  (const portCHAR *) "TaskI2C_SerialExt"
    ,  256 // This stack size can be checked & adjusted by reading Highwater
    ,  NULL
    ,  1  // priority
    ,  &xTaskI2C_SerialExt );
}
//***************************************************//

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

//***************************************************//

void TaskBlink(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  // initialize digital pin LED as an output.
  pinMode(LED, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
    digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second

    digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
  }
}
//***************************************************//


//***************************************************//
void TaskPrint(void *pvParameters)
{

  int element , i = 0;
  //char i2c_msg[16] = "";
  char *i2c_msg = "";
  //String i2c_str = "";
  //i2c_str.reserve(16);
  char c ;
  for (;;)
  {
    //xQueueReceive(queue, &element, portMAX_DELAY);
    if (xQueueReceive(queue, &element, portMAX_DELAY) )
    {

      float val = element * (3300 / 1023);

      if ( xSemaphoreTake( xMutex_print, portMAX_DELAY ) == pdTRUE )
      {
        Serial.println(val);
        xSemaphoreGive( xMutex_print );

      }


      if (val >= 900 && val <= 1100)
      {
        xTaskNotifyGive( xTaskBlink );
      }
    }
    //Serial.flush();

    if ( xSemaphoreTake( xMutex_print, portMAX_DELAY ) == pdTRUE )
    {
      if (Serial.available())
      {
        c = Serial.read();
        if (c != '\n')
        {
          i2c_msg[i++] = c;
          //i2c_str +=c;
        }
        else
        {
          i2c_msg[i++] = ';';
          i2c_msg[i] = '\0';
          //i2c_str += 0;
          xQueueSend(i2c_serial_ext_queue, &i2c_msg, portMAX_DELAY);
          //xQueueSend(i2c_serial_ext_queue, &i2c_str, portMAX_DELAY);
          i = 0;

        }
      }
      xSemaphoreGive( xMutex_print );
    }

  }

}
//***************************************************//

void TaskI2C(void *pvParameters)
{



  char element[16] ;
  char *str, c, str2[16] = {0} , *str3 = "77777777777" ;
  //String str;
  //char buff [16];
  int i = 0;
  int j = 3000;
  //str.reserve(16);



  for (;;)
  {
   // if ( xSemaphoreTake( xMutex_print, portMAX_DELAY ) == pdTRUE )
    //{

   
      if ( xSemaphoreTake( xMutex_i2c, portMAX_DELAY ) == pdTRUE )
      {
      Wire.requestFrom(7, 13);    // request 6 bytes from slave device #8

      while (Wire.available()) { // slave may send less than requested
        c = Wire.read(); // receive a byte as character
        if (c != -1)
        {
          Serial.print(c); // print the character
          *(str2 + i) = c;
          i++;
        }
      }
        xSemaphoreGive( xMutex_i2c );
      }


      *(str2 + i) = 0 ;
      i = 0 ;

      //Serial.print(str2);

      //strcpy(str3, str2);

      if (strncmp (str2, "79004C643D6C", 12) == 0 )
      {
        Serial.println("Sending U001 pattern");
        strcpy(str3, "U001;");
        Serial.println (str3);
        //         Serial.println("Sending U001 pattern**");

        xQueueSend(i2c_serial_ext_queue, &str3, portMAX_DELAY);


      }
      else if (strncmp (str2, "4D00234D183B", 12) == 0)
      {
        Serial.println("Sending U002 pattern");
        strcpy(str3, "U002;");
        Serial.println (str3);

        //Serial.println("Sending U002 pattern**");

        xQueueSend(i2c_serial_ext_queue, &str3, portMAX_DELAY);


      }
      else
      {
         Serial.println("no RFID matching PATTERN");
      }

      strcpy(str2 , "") ;
      //xSemaphoreGive( xMutex_print );
    //}



    vTaskDelay( 500 / portTICK_PERIOD_MS );

  }
}

//********************************************************//

void TaskI2C_SerialExt(void *pvParameters)
{
  char *str = "8888888888" ;

  for (;;)
  {
    //if ( xSemaphoreTake( xMutex_print, portMAX_DELAY ) == pdTRUE )
    //{
      
      if (xQueueReceive(i2c_serial_ext_queue, &str, ( TickType_t ) 50))

      { //xQueueSend(queue, &i, portMAX_DELAY);

        Serial.print("--"); Serial.println (str);
        if ( xSemaphoreTake( xMutex_i2c, portMAX_DELAY ) == pdTRUE )
        {
          Wire.beginTransmission(5);


          Wire.write(str);
          Wire.endTransmission();
          xSemaphoreGive( xMutex_i2c );
       }
      }
     
      //xSemaphoreGive( xMutex_print );

    //}

  }
}
