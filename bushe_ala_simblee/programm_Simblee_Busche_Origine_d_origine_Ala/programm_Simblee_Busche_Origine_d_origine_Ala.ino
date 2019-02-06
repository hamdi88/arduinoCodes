#include <SimbleeBLE.h>

//States
#define STATE_READY 1 //Waiting for button push
#define STATE_READING_RFID 2 //Reading the RFID
#define STATE_FIND_PROGRAM_CODE 3 //Finding the Program Code by BLE advertising
#define STATE_SEND_BARCODE 4 // Sending barcode to the Jukie Machine
#define STATE_PROCESSING 5 // Processing the article by the Jukie Machine
#define STATE_JIG_FINISHED_ADV 6 // Jig Finished Advertising

#define JUKIE_HANDSHAKE_BYTE 0x06
#define JUKIE_HANDSHAKE_BYTE_COUNT 7

#define RFID_LENGTH 12
#define TEST_RFID_DATA "HAMDIHOSNI"
const int FIND_PROGRAM_CODE_TIMEOUT = 5000; //Advertising Jig RFID Timeout
const int PROCESSING_TIMEOUT = 10000; //The Jukie processing timeout
const int JIG_FINISHED_ADV_TIMEOUT = 1000;

uint8_t state = STATE_READY;
int state_millis = 0;

int lastStatePrintTime = 0;

int const ledRed = 2;
int const ledGreen = 3;
int const ledBlue = 4;

int _lastLedRed = 0;
int _lastLedGreen = 0;
int _lastLedBlue = 0;

//int jukiProcessingState = 0; //added by houssem

boolean Jukie_Handshaked = false;

char RFID_data[RFID_LENGTH];

const int ADV_DATA_RFID_START_IDX = 13; //The index of the first char of the RFID that will be sent in the Service data
uint8_t RFID_adv_data[] =
{
  0x05, //Bluetooth Name
  0x09,
  0x42,
  0x53,
  0x43,
  0x48,
  
  
  0x02,  // length
  0x01,  // flags type
  0x06,  // le general discovery mode | br edr not supported

  0x0F,
  0x16,
  0x22,
  0x22,
  0x41,
  0x41,
  0x41,
  0x41,
  0x41,
  0x41,
  0x41,
  0x41,
  0x41,
  0x41,
  0x41,
  0x41,

  0x03,
  0x02,
  0x11,
  0x11
};

int const RFID_Button = 14;
int const RFID_trigg = 15; //Rfid relay trigger
int const JUKIE_FINISH_PIN = 20; //Pin 20 for Juki
int const Relay = 16;
int last_RFID_Button_State = 0;
char*  barcodeData;
boolean RFID_Reading_Active = false;

int lastJukiProcessingState = 0;

boolean jigAttached = false;
void Switch_State(int newState) ;
void setup()
{
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  
  pinMode(RFID_Button, INPUT);
  pinMode (RFID_trigg, OUTPUT);
  pinMode (Relay, OUTPUT);
  pinMode(JUKIE_FINISH_PIN, INPUT);

  //Serial.begin(9600);
}

void loop() 
{
  delay(100);

//  Handshake_Jukie();
Find_Program_Code(TEST_RFID_DATA);
//  if (Jukie_Handshaked == true)
//  switch (state)
//  {
//    case STATE_READY:
////      Toggle_RFID_Reader();
//    break;
//    case STATE_READING_RFID:
//      Read_RFID();
//    break;
//
//    case STATE_FIND_PROGRAM_CODE: {
//      if ((millis() - state_millis) > FIND_PROGRAM_CODE_TIMEOUT) {
//        SimbleeBLE.end();
//        Switch_State(STATE_READY);
//      }
//    }break;
//
//    case STATE_SEND_BARCODE: {
////      Send_Barcode();
//    }break;
//
//    case STATE_PROCESSING:
////      Wait_For_Juki_Processing();
//    break;
//
//    case STATE_JIG_FINISHED_ADV: {
//      if ((millis() - state_millis) > JIG_FINISHED_ADV_TIMEOUT) {
//        SimbleeBLE.end();
//        Switch_State(STATE_READY);
//      }
//    }break;
//  }
}

void SimbleeBLE_onAdvertisement(bool start){
}

void SimbleeBLE_onConnect() {
}

void SimbleeBLE_onReceive(char* data, int len) {
  if (state == STATE_FIND_PROGRAM_CODE) {
    barcodeData = data;
    Switch_State(STATE_SEND_BARCODE);
    
  } else if (state == STATE_JIG_FINISHED_ADV) {
    
    if (data[0] == 0xFF && data[1] == 0xFF) {
      delay(500);

      Switch_State(STATE_READY);
      SimbleeBLE.end();
    }
  }
}

void Read_RFID() {
  /*if (Serial.available() >= 13) {
    if (Serial.read() == 0x02) {
      for(int i = 0; i < RFID_LENGTH; i++) {
        RFID_data[i] = Serial.read();
      }
      
      Serial.flush();
      Serial.end();
      digitalWrite(RFID_trigg, LOW);
      Find_Program_Code(RFID_data);*/
     // Find_Program_Code(TEST_RFID_DATA);
    }
  


void Find_Program_Code(char* data) {
    
    for(int i = 0; i < RFID_LENGTH; i++) {
      RFID_adv_data[i + ADV_DATA_RFID_START_IDX] = data[i];
    }
    
   SimbleeBLE_advdata = RFID_adv_data;
   SimbleeBLE_advdata[27] = 0x11;
   SimbleeBLE_advdata[28] = 0x11,
   SimbleeBLE_advdata_len = sizeof(RFID_adv_data);
   SimbleeBLE.begin();
   Switch_State(STATE_FIND_PROGRAM_CODE);
}

void Send_Barcode() {
  int currentButtonState = digitalRead(RFID_Button); 

  if (last_RFID_Button_State == 0 && currentButtonState == 1) {
    digitalWrite(16, HIGH);
    delay(400);
    digitalWrite(16, LOW);
    jigAttached = true;
    Serial.begin(9600);
     for(int i = 0; i < sizeof(barcodeData); i++) {
      if (i == 1) {
        Serial.write(0x055);
      }
      Serial.write((byte) barcodeData[i]);
    }
    Serial.write(0x03);// added by Houssem (Copyright)
    Serial.end();
    Switch_State(STATE_PROCESSING);
  }
  last_RFID_Button_State = currentButtonState;
}

void Toggle_RFID_Reader() {
  delay(500);
  Switch_State(STATE_READING_RFID);
}

void Wait_For_Juki_Processing() {
  //Handle Jig unattaching
  int currentBracketBtnState = digitalRead(RFID_Button);

  if (last_RFID_Button_State == 0 && currentBracketBtnState == 1 && jigAttached == true) {
    digitalWrite(16, HIGH);
    delay(400);
    digitalWrite(16, LOW);
    jigAttached = false;
    Switch_State(STATE_READY);
    SimbleeBLE.end();
    return;
  }
  
  
  int jukiProcessingState = digitalRead(JUKIE_FINISH_PIN);
  if (jukiProcessingState != lastJukiProcessingState && jigAttached == true) {
    if (jukiProcessingState == HIGH) {
      //delay(200);
      SimbleeBLE.end();
      SimbleeBLE_advdata[27] = 0x22;
      SimbleeBLE_advdata[28] = 0x22,
      SimbleeBLE.begin();
      //delay(100);
      Switch_State(STATE_JIG_FINISHED_ADV);
    }
  }
  last_RFID_Button_State = currentBracketBtnState;
  lastJukiProcessingState = jukiProcessingState;
}

void Handshake_Jukie() {
  if (!Jukie_Handshaked) {
    Serial.begin(9600); //CPMMENTED BY HOUSSEM
    if (Serial.available()) {
      Serial.write(0x06);
      Serial.write(0x06);
      Serial.write(0x06);
      Serial.write(0x06);
      Serial.write(0x06);
      Serial.write(0x06);
      Serial.write(0x06);
      Jukie_Handshaked = true;
    }
  }
}

void Switch_State(int newState) {
  state = newState;
  state_millis = millis();
  
  switch (state) {
    case STATE_READING_RFID: {
      Set_Led_Color(0, 255, 0);
      Serial.flush();
      Serial.end();
      digitalWrite(RFID_trigg, HIGH);
      delay(1000);
      Serial.begin(9600, 9, 1);
    };break;


    case STATE_FIND_PROGRAM_CODE: {
      Set_Led_Color(0, 233, 179);
    };break;
    
    case STATE_PROCESSING: {
      Set_Led_Color(0, 0, 255);
    } break;

    case STATE_READY: {
      Set_Led_Color(0, 0, 0);
    }break;
  }

}
void Set_Led_Color(int r, int g, int b) {
  if (r != _lastLedRed) {
  //  analogWrite(ledRed, r);
    _lastLedRed = r; 
  }
  if (g != _lastLedGreen) {
   // analogWrite(ledGreen, g);
    _lastLedGreen = g; 
  }
  if (b != _lastLedBlue) {
   // analogWrite(ledBlue, b);
    _lastLedBlue = b;
  }
}
