#include <Adafruit_NeoPixel.h>
#include <Wire.h>

//slave variables
int id = 2;
const int dataLength = 6;
char data[dataLength];
long isActive = false;
//long complete = false;
//char nextRequest = 'X';

//LED strip variables
int ledStrip = 17;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, ledStrip, NEO_GRB + NEO_KHZ800);

//pin assignments
int submitButton = 14;
long submitPressed = false;

int fromOptions = 3; //should be 4
int fromButtonPins[] = {/*1,*/3,5,7};
int fromLedPins[] = {/*2,*/4,6,8};
char fromAnswer = 'X';

int withOptions = 3; //should be 4
int withButtonPins[] = {9,11,16/*,15*/};
int withLedPins[] = {10,12,15/*,16*/};
char withAnswer = 'X';

//int genderOptions = 3;
//int genderButtonPins[] = {17,18,19};
//int genderLedPins[] = {20,21,22};
//char genderAnswer = 'X';

void setup(){
  
  //setup LED strip
  strip.begin();
 
  //setup submit button
  pinMode(submitButton, INPUT);
  digitalWrite(submitButton, LOW);
  
  //setup FROM buttons and leds
  for(int i=0; i<fromOptions; i++){
    pinMode(fromButtonPins[i], INPUT);
    digitalWrite(fromButtonPins[i], LOW);
    pinMode(fromLedPins[i], OUTPUT);
    digitalWrite(fromLedPins[i], HIGH);
  }
  
  //setup WITH buttons
  for(int i=0; i<withOptions; i++){
    pinMode(withButtonPins[i], INPUT);
    digitalWrite(withButtonPins[i], LOW);
    pinMode(withLedPins[i], OUTPUT);
    digitalWrite(withLedPins[i], HIGH);
  }
  
  //setup GENDER buttons
//  for(int i=0; i<genderOptions; i++){
//    pinMode(genderButtonPins[i], INPUT);
//    digitalWrite(genderButtonPins[i], LOW);
//    pinMode(genderLedPins[i], OUTPUT);
//    digitalWrite(genderLedPins[i], LOW);
//  }
  
  Wire.begin(id);
  Wire.onReceive(handleEvent);
  Wire.onRequest(handleRequest);
  
  Serial.begin(9600);
  Serial.print("slave initialised with ID: ");
  Serial.println(id);
}

void loop(){
  if(isActive){
    
    //turn on LED strip
    turnOnStrip();
    
    //check for submit press
    if(digitalRead(submitButton) == HIGH){
      submitPressed = true;
    }
    
    //check for FROM presses
    for(int i=0; i<fromOptions; i++){
      if(digitalRead(fromButtonPins[i]) == HIGH){
        //turn them all off first
        for(int j=0; j<fromOptions; j++){
          digitalWrite(fromLedPins[j], HIGH);
        }
        //turn on selected LED
        digitalWrite(fromLedPins[i], LOW);
        fromAnswer = i+'0';
      }
    }
    
    //check for WITH presses
    for(int i=0; i<withOptions; i++){
      if(digitalRead(withButtonPins[i]) == HIGH){
        //turn them all off first
        for(int j=0; j<withOptions; j++){
          digitalWrite(withLedPins[j], HIGH);
        }
        //turn on selected LED
        digitalWrite(withLedPins[i], LOW);
        withAnswer = i+'0';
      }
    }
    
    //check for GENDER presses
//    for(int i=0; i<genderOptions; i++){
//      if(digitalRead(genderButtonPins[i]) == HIGH){
//        //turn them all off first
//        for(int j=0; j<genderOptions; j++){
//          digitalWrite(genderLedPins[j], HIGH);
//        }
//        //turn on selected LED
//        digitalWrite(genderLedPins[i], LOW);
//        genderAnswer = i+'0';        
//      }
//    }
  }else{
    submitPressed = false;
    
    //turn off LED strip
    turnOffStrip();
  
    //turn off all LEDS
    for(int i=0; i<fromOptions; i++){
      digitalWrite(fromLedPins[i], HIGH);
      fromAnswer = 'X';
    }
    for(int i=0; i<withOptions; i++){
      digitalWrite(withLedPins[i], HIGH);
      withAnswer = 'X';
    }
//    for(int i=0; i<genderResults; i++){
//      digitalWrite(genderLedPins[i], HIGH);
//      genderAnswer = 'X';
//    }

    //reset data
    memset(data, 0, dataLength);
  }
  
  delay(100);
}

void readInputs(){
  //get data from input devices
  //get age input
  data[1] = 'X';
  data[2] = 'X';
  data[3] = fromAnswer;
  data[4] = withAnswer;
  data[5] = 'X';
}

void handleEvent(int num){
  char event = Wire.read();
  
  Serial.print("Event received:");
  Serial.println(event);
  
  if(event == 'R'){  //RESET
    Serial.println("resetting slave...");
    isActive = false;
  }
}

void handleRequest(){
  Serial.println("Data request received...");
  
  isActive = true;
  
  if(submitPressed){
    //get data
    readInputs();
    Serial.println("returning Y with data");
    data[0] = 'Y';
    Wire.write(data);
    isActive = false;
  }else{
    Serial.println("returning N with null data");
    data[0] = 'N';
    Wire.write(data);
  }
}


/***************************
HELPER METHODS
***************************/
void turnOnStrip(){
  for(int i=0; i<strip.numPixels(); i++){
    strip.setPixelColor(i, strip.Color(0,255,0));  //GREEN
  }
  strip.show();
}
  
void turnOffStrip(){
  for(int i=0; i<strip.numPixels(); i++){
    strip.setPixelColor(i, strip.Color(0,0,0));
  }
  strip.show();
}
