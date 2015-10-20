#include <Adafruit_NeoPixel.h>
#include <Wire.h>

//slave variables
int id = 5;
const int dataLength = 2;
char data[dataLength];
long isActive = false;
long firstActive = true;
long reset = false;
//Keep state of phone box
/*
*  0 = preRing
*  1 = ringing
*  2 = rungout
*  3 = answered
*  4 = questioning
*  5 = hungup
*  6 = reset
*  7 = not hung up properly from last time
*  8 = short ring (not hungup last time)
*/
int phoneState = 0;

//LED strip variables
int ledStrip = 10;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, ledStrip, NEO_GRB + NEO_KHZ800);

//pin assignments
int submitButton = 9;
long submitPressed = false;
long pressTime = 0;

int cradlePin = 8;

int ringer = 7;
long startRingTime = 0;
long ringDuration = 16000;
long shortRingDuration = 8000;

int questions[] = {2, 3, 4, 5};  //T1, T2, T3, T4
long questionsDuration[] = {16000, 15000, 14000, 14000}; //in millis
long startQuestionTime = 0;
long currentQuestionDuration = 0;
int numQuestions = 4;
int currentQuestion = 0;

void setup(){
  
  //setup LED strip
  strip.begin();
  
  //turnOnStrip();
  data[1] = 'X';
 
  //setup submit button
  pinMode(submitButton, INPUT);
  digitalWrite(submitButton, LOW);
  
  //setup cradle pin
  pinMode(cradlePin, INPUT);
  digitalWrite(cradlePin, HIGH);
  
  //setup ringer pin
  pinMode(ringer, OUTPUT);
  digitalWrite(ringer, LOW);
  
  //setup question pins
  for(int i=0; i<numQuestions; i++){
    pinMode(questions[i], OUTPUT);
    digitalWrite(questions[i], LOW);
  }
  
  Wire.begin(id);
  Wire.onReceive(handleEvent);
  Wire.onRequest(handleRequest);
  
  Serial.begin(9600);
  Serial.print("slave initialised with ID: ");
  Serial.println(id);
} 

void loop(){
  if(isActive){
    
    if(firstActive){
      Serial.println("Checking if phone properly hung up from last time...");
      //check if phone not hung up properly from last time
      if(digitalRead(cradlePin) == LOW){
        phoneState = 7;
      }
      firstActive = false;
    }
    
    //turn on LED strip
    turnOnStrip();
    
    //check for submit press
    if(digitalRead(submitButton) == HIGH){
      //if((millis()-pressTime) > 5000){ //it's been more than 5 seconds since the last submit press
        submitPressed = true;
        Serial.println("submit pressed");
        //Serial.println(millis());
        //pressTime = millis();
      //}
    }
     
    else{ 
      //go through phone states
      if(phoneState == 0){ //preRing
        //ring phone
        Serial.println("phone state = 0: start ringing...");
        delay(500);
        startRinging();
        startRingTime = millis();
        //delay(50);
        //Serial.print("start ring time = ");
        //Serial.println(startRingTime);
        phoneState = 1;
      }
      
      else if(phoneState == 1){ //ringing
        //check if rungout
        //Serial.println("Phone state = 1: ringing in progress...");
        if((millis() - startRingTime) > ringDuration){ //been ringing for 16 seconds
          //rungout - act like submit has been pressed
          phoneState = 2;
          submitPressed = true;
          Serial.println("Phone state = 2: phone has rung out");
        }else{
          //check if answered
          if(digitalRead(cradlePin) == LOW){
            //phone answered
            Serial.println("phone answered...");
            stopRinging();
            //delay(50);
            phoneState = 3;
          }
        }
      }
      
      else if(phoneState == 3){ //answered
        //play question - iterate through list
        Serial.println("phone state = 3: start question...");
        //delay(300);
        playQuestion(currentQuestion);
        startQuestionTime = millis();
        //delay(50);
        
        currentQuestionDuration = questionsDuration[currentQuestion];
        phoneState = 4;
      }
      
      else if(phoneState == 4){ //questioning
        //Serial.println("Phone state = 4: asking question...");
        //check if question finished
        if((millis() - startQuestionTime) > currentQuestionDuration){
          phoneState = 5;
        }
        //check if hungup
        if(digitalRead(cradlePin) == HIGH){
          //hungup
          phoneState = 5;
          Serial.println("phone state = 5: hung up");
          //stop question if still asking
          if((millis() - startQuestionTime) < currentQuestionDuration){
            stopQuestion();
            //delay(50);
          }
        }
      }
      
      else if(phoneState == 5){
//        currentQuestion++;
//        if(currentQuestion > 4){
//          currentQuestion = 0;
//        }
        phoneState = 6;
      }
      
      else if(phoneState == 7){ //not hung up properly from last time
        //ring for 10 seconds
        Serial.println("phone state = 7: do short ring...");
        startRinging();
        startRingTime = millis();
        phoneState = 8;
      }
      
      else if(phoneState == 8){
        if((millis() - startRingTime) > shortRingDuration){ //has been ringing for 10 seconds
          stopRinging();
          phoneState = 3;
        }
      }
    }
  
  }else{
    
    if(reset){
      //stop ringing or questioning if happening
      //Serial.print("phoneState = ");
      //Serial.println(phoneState);
      if(phoneState == 1){
        Serial.println("stopping ring...");
        phoneState = 6;
        stopRinging();
        //delay(50);
      }
      if(phoneState == 4){
        Serial.println("stopping question...");
        phoneState = 6;
        stopQuestion();
        //delay(50);
      }
      currentQuestion++;
      //Serial.print("current question after increment = ");
      //Serial.println(currentQuestion);
      if(currentQuestion >= numQuestions){
        currentQuestion = 0;
      }
      //Serial.print("current question after IF statement = ");
      //Serial.println(currentQuestion);
      reset = false;
    }
    
    firstActive = true;
    submitPressed = false;
    
    //turn off LED strip
    turnOffStrip();
    
    phoneState = 0;
    startRingTime = 0;
    startQuestionTime = 0;
  }
  
  delay(100);
}

void handleEvent(int num){
  char event = Wire.read();
  
  //Serial.print("Event received:");
  //Serial.println(event);
  
  if(event == 'P'){
    isActive = true;
  }
  
  //if(event == 'R'){  //RESET
    //Serial.println("resetting slave...");
    //isActive = false;
  //}
}

void handleRequest(){
  //Serial.print("Data request received at: ");
  //Serial.println(millis());
  
  isActive = true;
  
  if(submitPressed){
    //get data
    //readInputs();
    //Serial.println("returning Y with data");
    data[0] = 'Y';
    Wire.write(data);
    isActive = false;
    reset = true;
  }else{
    //Serial.println("returning N with null data");
    data[0] = 'N';
    Wire.write(data);
  }
}


///***************************
//HELPER METHODS
//***************************/
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

void startRinging(){
  digitalWrite(ringer, HIGH);
  delay(50);
  digitalWrite(ringer, LOW);
}

void stopRinging(){
  startRinging();
}

void playQuestion(int question){
  digitalWrite(questions[question], HIGH);
  delay(50);
  digitalWrite(questions[question], LOW);
}

void stopQuestion(){
  playQuestion(0);
}
