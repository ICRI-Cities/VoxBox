#include <math.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>

//slave variables
int id = 4;
const int dataLength = 3;
char data[dataLength];
long isActive = false;


/*Sarah additions*/
char roll1Answer = 'X';
char roll2Answer = 'X';

//LED strip variables
int ledStrip = 2;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, ledStrip, NEO_GRB + NEO_KHZ800);

//pin assignments
int submitButton = 3;
int rollerpins[2] = {A1, A2};
long submitPressed = false;


void setup() {

  //setup LED strip
  strip.begin();
  //turnOnStrip();
  //setup submit button
  pinMode(submitButton, INPUT);
  digitalWrite(submitButton, LOW);
  
  Wire.begin(id);
  Wire.onReceive(handleEvent);
  Wire.onRequest(handleRequest);
  
  Serial.begin(9600);
  Serial.print("slave initialised with ID: ");
  Serial.println(id);
}

void loop() {
  if(isActive){
    
    //turn on LED strip
    turnOnStrip();
    
    //check for submit press
    //Serial.println(digitalRead(submitButton));
    if(digitalRead(submitButton) == HIGH){
      Serial.println("Button pressed");
      submitPressed = true;
    }else{
      //check roller 1
      int val = analogRead(rollerpins[0]);
      //Serial.print("roller 1: ");
     // Serial.println(val);
      if (val < 120)
        roll1Answer = '0';
      else if (val >= 120 && val < 375)
        roll1Answer = '1';
      else if (val >= 375 && val < 650)
        roll1Answer = '2';
      else if (val >= 650 && val < 905)
        roll1Answer = '3';
      else
        roll1Answer = '4';
      //Serial.print("roller 1: ");
      //Serial.println(roll1Answer);
      
               
      //check roller 2
      val = analogRead(rollerpins[1]);
      //Serial.print("roller 2: ");
      //Serial.println(val);
      if (val < 120)
        roll2Answer = '0';
      else if (val >= 120 && val < 375)
        roll2Answer = '1';
      else if (val >= 375 && val < 650)
        roll2Answer = '2';
      else if (val >= 650 && val < 905)
        roll2Answer = '3';
      else
        roll2Answer = '4';
      //Serial.print("roller 2: ");
      //Serial.println(roll2Answer);
        
     
    }
    
 
  }
  else
  {
    //Serial.println("inside else");
    
    submitPressed = false;
    
    //turn off LED strip
    turnOffStrip();
    
    //reset data
    memset(data, 0, dataLength);
  
  }
  
  delay(100);

}


void readInputs(){
  //get data from input devices
  
  data[1] = roll1Answer; //rolleranswers[0];
  data[2] = roll2Answer; //rolleranswers[1];
  
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
  
  //initial reading
  /*for (int i=0; i < 5; i++)
    {
      rollervals[i] = analogRead(rollerpins[i]);    // read the value from the sensor
      storeData(i, rollervals[i]);
      prevvals[i] = rollervals[i]; 
    }*/
  
  if(submitPressed){
    //get data
    readInputs();
    Serial.println("returning Y with data");
    data[0] = 'Y';
    Wire.write(data);
    isActive = false;
    
    //reset rollers
    //for(int i=0; i<5; i++){
      //rolleranswers[i] = 'X';
    //}
    
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


