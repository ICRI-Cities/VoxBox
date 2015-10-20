// Servos: makes the servos move one by one after receiving a message from the Master to let the ball through

#include <Servo.h> 
#include <Wire.h>
 
Servo servos[6];  

int startpos = 70; 
int movepos = 30;    // variable to store the servo position 
const int buttonPin = 2;
int curservo = 0;
long lastservo = false;
 
void setup() 
{ 
  Serial.begin(9600);
 
  pinMode(buttonPin, INPUT);     
  servos[0].attach(3);  
  servos[1].attach(5);
  servos[2].attach(6);
  servos[3].attach(9);
  servos[4].attach(10);
  servos[5].attach(11);
  
  for(int i=0; i < 6; i++)
  {
     servos[i].write(startpos);       //set all servo position to the basis position at the start 
     delay(200);
  }
  
  //delay(3000); //to ensure master has started
  
  Wire.begin(666);
  Wire.onReceive(handleEvent);
  
  Serial.println("servos ready");
} 


void loop_old(){
  for(int i=0; i<6; i++){
    servos[i].write(movepos);
    delay(10000);
  }
   for(int i=0; i < 6; i++)
  {
     servos[i].write(startpos);       //set all servo position to the basis position at the start 
     delay(200);
  }
  delay(10000);
}
 
 
void loop() 
{ 
  if (lastservo == true)
  {
    delay(500);
    for(int i=0; i < 6; i++)
    {   
      servos[i].write(startpos);
      delay(200);
    }
    lastservo = false;
  }
}

void handleEvent(int num){
  char event = Wire.read();
  
  Serial.print("Event received:");
  Serial.println(event);
  
  curservo = event-'0';
  Serial.print("id received, moving servo: ");
  Serial.println(curservo);
  servos[curservo].write(movepos);
  if (curservo == 5)
    lastservo = true;
 
}
