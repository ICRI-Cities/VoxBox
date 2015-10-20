/*
A note on setting the output sequence:
Think of each LED as a single bit in a 16-bit long binary string.  If a bit is 1, the LED is on, if a bit is 0, the LED is off.  
By making a string of ones and zeros, we choose which LEDs to have on and off, and then send this string to the shift register to display it.
For example 1000000000000001 binary (0x8001 in hex) will have the fist and last LEDs on, the rest off.  
*/

#include <Adafruit_NeoPixel.h>
#include <Wire.h>


//slave variables
int id = 3;
const int dataLength = 7;
char data[dataLength];
long isActive = false;
boolean firstActive = false;

int potvals [6] = {0,0,0,0,0,0};
char potanswers [6] = {'X','X','X','X','X','X'};

//LED strip variables
int ledStrip = 17;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, ledStrip, NEO_GRB + NEO_KHZ800);

//pin assignments
int submitButton = 16;
long submitPressed = true;

int ENC_A[3] = {15,49,53};
int ENC_B[3] = {14,48,52};
int SDI[3] =   {23,29,35};
int CLK[3] =   {25,31,37};
int LE[3] =    {27,33,39};
#define ENC_PORT_1 PINJ
#define ENC_PORT_2 PINL
#define ENC_PORT_3 PINB

// Global variables
int scaledCounter[3] = {0,0,0};
int maxCounter = 35; //at this number the LEDs turn on at the same speed as the knob spins
int halfCounter = 19;

unsigned int sequence[16] = {0x0,0x1,0x3,0x7,0xf,0x1f,0x3f,0x7f,0xff,0x1ff,0x3ff,0x7ff,0xfff,0x1fff,0x3fff,0x7fff};

void setup()
{    
  //Set SPI pins to output
  //setup LED strip
  strip.begin();
 
  //setup submit button
  pinMode(submitButton, INPUT);
  digitalWrite(submitButton, LOW);
  
  for (int i=0; i<3; i++)
  {
    //Set led pins to output
    pinMode(SDI[i], OUTPUT);
    pinMode(CLK[i], OUTPUT);
    pinMode(LE[i],OUTPUT);
    
    //Set encoder pins to input, turn internal pull-ups on
    pinMode(ENC_A[i], INPUT);
    digitalWrite(ENC_A[i], HIGH);
    pinMode(ENC_B[i], INPUT);
    digitalWrite(ENC_B[i], HIGH);

    //set all LEDs off for start position
    digitalWrite(LE[i],LOW);
    shiftOut(SDI[i],CLK[i],MSBFIRST,(sequence[0] >> 8));    //High byte first
    shiftOut(SDI[i],CLK[i],MSBFIRST,sequence[0]);           //Low byte second
    digitalWrite(LE[i],HIGH);
    
    Wire.begin(id);
    Wire.onReceive(handleEvent);
    Wire.onRequest(handleRequest);
      
    Serial.begin(9600);
    Serial.print("slave initialised with ID: ");
    Serial.println(id);
    
    initLights();
  }
}


void loop()
{ 
  //Local Variables
  
  static uint8_t counter[3] = {halfCounter,halfCounter,halfCounter}; //these variables will be changed by encoder input
  int8_t tmpdata [3];
  if(isActive){
    
    //turn on LED strip
    turnOnStrip();
    
    //check for submit press
    if(digitalRead(submitButton) == HIGH){
      Serial.println("Button pressed");
      submitPressed = true;
    }
  
  
    //Call read_encoder() function 
    for (int i=0; i<3; i++)
    {  
      if (i == 0)
        tmpdata[i] = read_encoder_1();
      else if (i == 1)
        tmpdata[i] = read_encoder_2();
      else
        tmpdata[i] = read_encoder_3();
      //if ENCODER has moved
      if(tmpdata[i]) 
      {
          
          //make sure that the counter stays in range so that the knob is not turned endlessly out of range
          if (counter[i]+tmpdata[i] < maxCounter)
          {
            //make sure that the counter stays in range so that the knob is not turned endlessly out of range
            if (counter[i]+tmpdata[i] > 0)
            {
              //Set the new counter value of the encoder
               counter[i] += tmpdata[i];
            }
            else
            {
               counter[i] = 0;
            }   
          }
          else
          {
             counter[i] = maxCounter;
          }   
          
          
          //Scale the counter value for referencing the LED sequence
          //***NOTE: Change the map() function to suit the limits of your rotary encoder application.
          //         The first two numbers are the lower, upper limit of the rotary encoder, the 
          //         second two numbers 0 and 14 are limits of LED sequence arrays.  This is done
          //         so that the LEDs can use a different scaling than the encoder value. 
          scaledCounter[i] = map(counter[i],0,maxCounter,0,15);
          //Serial.println(scaledCounter[i]);
          //Send the LED output to the shift register 
          digitalWrite(LE[i],LOW);
          shiftOut(SDI[i],CLK[i],MSBFIRST,(sequence[scaledCounter[i]] >> 8));    //High byte first
          shiftOut(SDI[i],CLK[i],MSBFIRST,sequence[scaledCounter[i]]);           //Low byte second
          digitalWrite(LE[i],HIGH);
          
          storeData(i, scaledCounter[i]);
          
        }
    }
  }
  else
  {
    submitPressed = false;
    
    
    //turn off LED strip
    turnOffStrip();
    lightsOff();
    
    //reset data
    memset(data, 0, dataLength);
  }
  
  delay(14);

}

//initialises the rotaries to start in the middle of the range
void initLights()
{
  for (int i = 0; i<3; i++)
  {
     scaledCounter[i] = 8; 
     //Serial.println(scaledCounter[i]);
     //Send the LED output to the shift register 
     digitalWrite(LE[i],LOW);
     shiftOut(SDI[i],CLK[i],MSBFIRST,(sequence[scaledCounter[i]] >> 8));    //High byte first
     shiftOut(SDI[i],CLK[i],MSBFIRST,sequence[scaledCounter[i]]);           //Low byte second
     digitalWrite(LE[i],HIGH);
     storeData(i, scaledCounter[i]);
  }
          
    
  
}

void lightsOff()
{
  for (int i = 0; i<3; i++)
  {
     scaledCounter[i] = 0; 
     //Serial.println(scaledCounter[i]);
     //Send the LED output to the shift register 
     digitalWrite(LE[i],LOW);
     shiftOut(SDI[i],CLK[i],MSBFIRST,(sequence[scaledCounter[i]] >> 8));    //High byte first
     shiftOut(SDI[i],CLK[i],MSBFIRST,sequence[scaledCounter[i]]);           //Low byte second
     digitalWrite(LE[i],HIGH);
  } 
  firstActive = false;
  
}

void storeData (int cur, int value)
{
  //Serial.print("storing data: ");
  //Serial.println(value);
  int digit1 = value/10;
  int digit2 = value - (digit1*10);

  if (cur == 0)
  {
    potvals[0] = digit1;
    potvals[1] = digit2;
  }else if (cur == 1)
  {
    potvals[2] = digit1;
    potvals[3] = digit2;
  }else
  {
    potvals[4] = digit1;
    potvals[5] = digit2;
  }/*
  for (int i=0; i<6; i++)
  {
    Serial.print("digit ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(potanswers[i]);
  }*/
  for (int i=0; i<6;i++)
  {
    potanswers[i] = potvals[i]+'0';
    
  }
}

/*************************************************************************
*    read_encoder() function as provided by Oleg:                        *
*    http://www.circuitsathome.com/mcu/reading-rotary-encoder-on-arduino *
*                                                                        *
*    Returns change in encoder state (-1,0,1)                            *
************************************************************************ */
int8_t read_encoder_1()
{
  int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
  static uint8_t old_AB = 0;
  /**/
  old_AB <<= 2;                   //remember previous state
  old_AB |= ( ENC_PORT_1 & 0x03 );  //add current state
  return ( enc_states[( old_AB & 0x0f )]);
}

int8_t read_encoder_2()
{
  int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
  static uint8_t old_AB = 0;
 
  old_AB <<= 2;                   //remember previous state
  old_AB |= ( ENC_PORT_2 & 0x03 );  //add current state
  return ( enc_states[( old_AB & 0x0f )]);
}
int8_t read_encoder_3()
{
  int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
  static uint8_t old_AB = 0;
  old_AB <<= 2;                   //remember previous state
  old_AB |= ( ENC_PORT_3 & 0x03 );  //add current state
  return ( enc_states[( old_AB & 0x0f )]);
}

void readInputs(){
  //get data from input devices
  
  data[1] = potanswers[0];
  data[2] = potanswers[1];
  data[3] = potanswers[2];
  data[4] = potanswers[3];
  data[5] = potanswers[4];
  data[6] = potanswers[5];
}

void handleEvent(int num){
  char event = Wire.read();
  
  Serial.print("Event received:");
  Serial.println(event);
  
  if(event == 'R'){  //RESET
    //Serial.println("resetting slave...");
    isActive = false;
  }
}

void handleRequest(){
  Serial.println("Data request received...");
  
  isActive = true;
  if (firstActive == false)
  {
   initLights(); 
   firstActive = true;
  }
  
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



