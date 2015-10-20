/****
* By Sarah Gallacher
* 
* Updated 26/08/2015
*****/

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>

/****************************
*
* CHANGE WIFI DETAILS HERE
*
*****************************/
//wifi variables
char ssid[] = "CS66GS";              //CHANGE THIS TO NETWORK SSID
char password[] = "WIRELESS_PETE";   //CHANGE THIS TO PASSWORD
int status = WL_IDLE_STATUS;

//server variables
WiFiClient client;
char server[] = "icri-nodejs.cs.ucl.ac.uk";
int port = 4321;
long pingDelay = 10000;
long lastPing = 0;

long connectedMode = false;

//master variables
boolean inProgress = false;
char dataInstance[300];  //BUFFER SIZE OF LOCAL DATASTORAGE = 300;
int instanceIndex = 1;

//slave variables
const int maxSlaves = 10;  //MAX NUMBER OF SLAVES
const int maxListResponse = 60; //maxSlaves*6
char response[60]; //[maxListResponse];
int index = 0;

int slaves[maxSlaves][2];     //[ID, response length including boolean value]
int numSlaves = 0; 
int currentSlave = 0;
int servos[] = {1, 2, 3, 4, 5};

//pin assignments
int startLED = 5;
int startButton = 4;
long startOn = false;
long startPressed = false;
long flashDelay = 400;
long lastFlash = 0;
long startPressEnd = 0;

void setup(){
  //set up I2C comms with slave arduinos  
  Wire.begin();
  
  Serial.begin(9600);
  Serial.println("setup...");
  
  //setup LED and button
  pinMode(startLED, OUTPUT);
  digitalWrite(startLED, HIGH);
  
  pinMode(startButton, INPUT);
  digitalWrite(startButton, LOW);
  
  //clear data instance
  memset(dataInstance, 0, sizeof(dataInstance));
  dataInstance[0] = 'I';
  
  startConnection();
  
  //INITIALISE - request slave list
  if(connectedMode){
    if(client.connected()){
      requestSlaveList();
    }
  }
}

void loop(){
  //handle waiting for next voxbox cycle
  if(!inProgress){
    
    //ping server
    if(connectedMode){
      if(client.connected()){
        if(millis() - lastPing > pingDelay){
          pingServer();
          lastPing = millis();
        }
      }
    }
    
    //flash start button
    //check if start button is pushed
    if(digitalRead(startButton) == HIGH){
      //Press just started
      if(!startPressed && (millis()-startPressEnd)>1000){
        Serial.println("start button pressed");
        //startPressBegin = millis();
        startPressed = true;
      }
    }else{
      //Press just finished
      if(startPressed){
        Serial.println("start button press ended");
        startPressEnd = millis();
        startPressed = false;
        //turn off button LED
        digitalWrite(startLED, HIGH);
        startOn = false;
        //move onto next part
        inProgress = true;
        
        //send message to servos
        Serial.println("sending 0 message to servos");
        
        Serial.print("control message to servos: ");
        Serial.println('0');
        
        Wire.beginTransmission(666);
        Wire.write('0');
        Wire.endTransmission();
        
      }else{
        //No press
        //FLASH start button
        if(startOn){
          if(millis() - lastFlash > flashDelay){
            //Serial.println("turning led off");
            digitalWrite(startLED, LOW);
            startOn = false;
            lastFlash = millis();
          }
        }else{
          if(millis() - lastFlash > flashDelay){
            //Serial.println("turning led on");
            digitalWrite(startLED, HIGH);
            startOn = true;
            lastFlash = millis();
          }
        }
      }
    }
 
  }else{
    
    //handle voxbox cycle
    
    //turn off LED start button
    digitalWrite(startLED, HIGH);
    startOn = false;
    
    //activate current slave if not already active
    if(currentSlave < numSlaves){ //still not reached end of all the slaves
      requestSlaveUpdate(slaves[currentSlave][0], slaves[currentSlave][1]);
      delay(500);
    }else{
      //reached end of all the slaves
      //send dataInstance to server and reset
      if(connectedMode){
        sendDataToServer();
      }
      
      //reset eveything
      currentSlave = 0;
      memset(dataInstance, 0, sizeof(dataInstance));
      dataInstance[0] = 'I';
      instanceIndex = 1;
      inProgress = false;
    }
  }
}

/***********************
Comms with slave arduinos via WIRE
***********************/

char requestSlaveUpdate(int slaveID, int responseLength){
  //request update from current slave
  //send WIRE request
  Wire.requestFrom(slaveID, responseLength); //request 1 byte from the current slave
  
  //wait for response
  char response[responseLength]; 
  int index = 0;
  while(Wire.available()){
    response[index] = Wire.read();
    index++;
  }
  
  if(response[0] == 'Y'){ //slave is finished          
    //read data into master store
    //add response to dataInstance
    Serial.print("Slave data = ");
    for(int i=1; i<responseLength; i++){
      dataInstance[instanceIndex] = response[i];
      instanceIndex++;
      Serial.print(response[i]);
    }
    Serial.println();
    
    //send message to servos
    int servoId = servos[currentSlave];
    
    Serial.print("control message to servoId: ");
    Serial.println(servoId);
    
    Wire.beginTransmission(666);
    Wire.write(servoId+'0');  //change to char
    Wire.endTransmission();
    
    currentSlave++; 
  }
}


/****************************
Comms with server via WiFi
****************************/
void requestSlaveList(){
  Serial.println("Requesting slave list from server...");
  long gotResponse = false;
  
  //send request
  client.write("L");  //L = request slave list
  
  //wait for response
  char response[maxListResponse];
  int index = 0;
  Serial.println("Waiting for response from server...");
  while(!gotResponse){
    //Serial.println("waiting....");
    if(client.available()){
      //Serial.println("got something");
      char c = client.read();
      //Serial.print(c);
      if(c != '\n'){
        response[index] = c;
        //Serial.print(response[index]);
        index++;
      }else{
        //Serial.println();
        Serial.println("received response from server...");
        gotResponse = true;
      }
    }
  }
  
  //process response
  Serial.println("Processing slave response...");
  int responseLength = index;
  Serial.print("Response length = ");
  Serial.println(responseLength);
  
  //populate slaves arrays
  int i = 0; //response index
  int j = 0; //slaves array index
  while(i < responseLength){ //e.g. 001 005 002 011
    int hid = response[i] - '0'; i++;
    int tid = response[i] - '0'; i++;
    int uid = response[i] - '0'; i++;
    int hlength = response[i] - '0'; i++;
    int tlength = response[i] - '0'; i++;
    int ulength = response[i] - '0'; i++;
    int id = toInteger(hid, tid, uid);
    int length = toInteger(hlength, tlength, ulength);
    
    slaves[j][0] = id;
    slaves[j][1] = length;
    j++; //increase for next entry
    numSlaves++; //keep count of number of slaves
  }
}


void sendDataToServer(){
  //Serial.println("sending final data from all slaves to server...");
  
  dataInstance[instanceIndex] = '\0';
  Serial.print("FINAL DATA = ");
  Serial.println(dataInstance);
  
  if(status == WL_CONNECTED){
    if(client.connected()){
      Serial.println("uploading...");
       client.println(dataInstance);
    }else{
      Serial.println("reconnecting...");
      client.flush();
      client.stop();
      client.connect(server, port);
      
      if(client.connected()){
        Serial.println("uploading...");
        client.println(dataInstance);
      }else{
        Serial.println("Lost connection to server");
         serverError();
      }
    }
  }else{
    Serial.println("Lost wifi connection");
    wifiError();
  }
}

void pingServer(){
  Serial.println("pinging server...");
  client.write("p");
}

/******************************
WiFi and server connection methods
******************************/
void startConnection(){
  if(connectToWifi()){
    if(connectToServer()){
      connectedMode = true;
      return;
    }else{
      //show server connection error blinks
      serverError();
    }
  }else{
    //show wifi connection error blinks
    wifiError();
  }
  //for demo purposes
  Serial.println("Setting default slaves...");
  setDefaultSlaves();
}

boolean connectToWifi(){
  int tries = 0;
  while(status != WL_CONNECTED && tries < 10){
    
    Serial.print("Connecting to wifi. Attempt ");
    Serial.print(tries+1);
    Serial.println("...");
    
    status = WiFi.begin(ssid, password);
    delay(5000);
    tries++;
  }
  
  if(status != WL_CONNECTED){
    Serial.println("Connection to wifi failed");
    return false;
  }else{
    Serial.println("connected to wifi");
    Serial.println(WiFi.localIP());
  }
  return true;
}

boolean connectToServer(){
  int tries = 0;
  while(!client.connected() && tries < 10){
    Serial.print("Connecting to server. Attempt ");
    Serial.print(tries+1);
    Serial.println("...");
    
    client.connect(server, port);
    tries++;
  }
  
  if(!client.connected()){
     Serial.println("Connection to server failed");
    return false;
  }else{
    Serial.println("connected to server");
  }
  return true;
}

//these may not be the connected slaves
//we assume so for now so Voxbox will still operate under demo situations
void setDefaultSlaves(){
  slaves[0][0] = 1; slaves[0][1] = 5;
  slaves[1][0] = 3; slaves[1][1] = 6;
  slaves[2][0] = 2; slaves[2][1] = 7;
  slaves[3][0] = 4; slaves[3][1] = 3;
  slaves[4][0] = 5; slaves[4][1] = 2;
  numSlaves = 5;
}

void serverError(){
  //flash LED 6 times fast
  for(int i=0; i<6; i++){
    digitalWrite(startLED, HIGH);
    delay(100);
    digitalWrite(startLED, LOW);
    delay(100);
  }
}

void wifiError(){
  //flash LED 3 times fast
  for(int i=0; i<3; i++){
    digitalWrite(startLED, HIGH);
    delay(100);
    digitalWrite(startLED, LOW);
    delay(100);
  }
}


/**************************************
Helper methods
**************************************/

int toInteger(int hundreds, int tens, int units){
  return (hundreds*100)+(tens*10)+units;
}


/*void reconnectToServer(){
  if(!client.connected()){
    if(!firstConnection){
      client.flush();
      client.stop();
    }
    Serial.print("Connecting to server. Attempt ");
    Serial.print(tries+1);
    Serial.println("...");
    
    client.connect(server, port);
    firstConnection = false;
    tries++;
  }
}*/
