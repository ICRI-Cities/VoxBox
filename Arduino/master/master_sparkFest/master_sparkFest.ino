/****
* By Sarah Gallacher
* 
* Updated 11/08/2015
*****/

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <avr/pgmspace.h>
#include <Adafruit_Thermal.h>
#include "beaker.h"
#include "computer.h"
#include "crane.h"
#include "electric.h"
#include "gear.h"
#include "rocket.h"
#include "papercut.h"
//#include "question.h"
//#include "subtext.h"

//FLASH VARIABLES
const char header[] PROGMEM = "are you?        engineer        What kind of    Spark Festival: ";
const char subtext[] PROGMEM = "result...       calculated your VoxBox has      given, the      answers you haveBased on the    ";
const char answer_start[] PROGMEM = "about becoming aHave you thought";
const char answer_end[] PROGMEM = " Engineer";
const char url[] PROGMEM = "www.voxboxproject.com/spark";

const char mech[] PROGMEM = "Mechanical";
const char elec[] PROGMEM = "Electrical";
const char aero[] PROGMEM = "Aerospace";
const char bio[] PROGMEM = "Bio-chemical";
const char soft[] PROGMEM = "Software";
const char civil[] PROGMEM = "Civil";
const char* const engineers[] PROGMEM = {mech,elec,aero,bio,soft,civil};
char buffer[20];

const uint16_t dialAnswers[] PROGMEM = {0,1,2,3,4,5,2,4,5,3,1,0,5,3,4,0,2,1};
const uint16_t sliderAnswers[] PROGMEM = {4,3,5,2,1,0,0,1,2,4,3,5,5,3,1,4,2,0,5,2,0,1,3,4,5,4,1,0,2,3};
const uint16_t spinnerAnswers[] PROGMEM = {5,3,2,1,0,4,2,0,3,1};


//wifi variables
//char ssid[] = "CS66GS";
char ssid[] = "_FreeParkWiFi";
//char password[] = "WIRELESS_PETE";
int status = WL_IDLE_STATUS;

//server variables
WiFiClient client;
char server[] = "icri-nodejs.cs.ucl.ac.uk";
int port = 4321;
long pingDelay = 10000;
long lastPing = 0;

//mode variable
long connectedMode = false;

//printer variables
SoftwareSerial printerSerial(2, 3); //RX, TX
Adafruit_Thermal Printer(&printerSerial);

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

//algorithm variables
int answerCount[] = {0,0,0,0,0,0};

void setup(){
  //set up I2C comms with slave arduinos  
  Wire.begin();
  
  Serial.begin(9600);
  
  //setup printer
  printerSerial.begin(19200);
  Printer.begin();
  //print upside down
  printerSerial.write(27);
  printerSerial.write(123);
  printerSerial.write(1);
  Printer.setSize('L');
  Serial.println(F("Printer setup..."));
  
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
        //Serial.println("start button pressed");
        //startPressBegin = millis();
        startPressed = true;
      }
    }else{
      //Press just finished
      if(startPressed){
        //Serial.println("start button press ended");
        startPressEnd = millis();
        startPressed = false;
        //turn off button LED
        digitalWrite(startLED, HIGH);
        startOn = false;
        //move onto next part
        inProgress = true;
        
        //send message to servos
        //Serial.println("sending 0 message to servos");
        
        //Serial.print("control message to servos: ");
        //Serial.println("0");
        
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
      //if(client.connected()){
        //run algorithm
        int result = calculateResult();
        printResult(result);
        
        if(connectedMode){
          sendDataToServer();
        }
      //}
      
      //reset eveything
      currentSlave = 0;
      memset(answerCount, 0, sizeof(answerCount));
      memset(dataInstance, 0, sizeof(dataInstance));
      dataInstance[0] = 'I';
      instanceIndex = 1;
      inProgress = false;
    }
  }
}

/***********************
Algorithm to calculate result of engineer type
***********************/
int calculateResult(){
  
  parseDials();
  parseSliders();
  parseSpinners();
  
  //get most popular engineer
  int maxCount = 0;
  int maxIndex = 0;
  for(int i=0; i<6; i++){
    int nextCount = answerCount[i];
    Serial.println(nextCount);
    if(nextCount > maxCount){
      maxCount = nextCount;
      maxIndex = i;
    }
  }
  
  dataInstance[instanceIndex] = maxIndex + '0';
  instanceIndex++;
  
  return maxIndex;
}

void parseDials(){  
  int dataIndex = 5; //pointer to starting index in dataInstance
  for(int i=0; i<3; i++){
    int tInt = dataInstance[dataIndex++] - '0';
    int uInt = dataInstance[dataIndex++] - '0';
    int answer = (10 * tInt) + uInt;
    unsigned int countIndex;
    if(answer<3){
      countIndex = pgm_read_byte(&(dialAnswers[i*6]));
    }else if(answer>=3 && answer<6){
      countIndex = pgm_read_byte(&(dialAnswers[(i*6)+1]));
    }else if(answer>=6 && answer<9){
      countIndex = pgm_read_byte(&(dialAnswers[(i*6)+2]));
    }else if(answer==9){
      countIndex = pgm_read_byte(&(dialAnswers[(i*6)+3]));
    }else if(answer>=10 && answer<13){
      countIndex = pgm_read_byte(&(dialAnswers[(i*6)+4]));
    }else{
      countIndex = pgm_read_byte(&(dialAnswers[(i*6)+5]));
    }
    int tmp = answerCount[countIndex];
    tmp = tmp+1;
    answerCount[countIndex] = tmp;
    
    //debugging
    //Serial.print("dial countIndex = ");
    //Serial.println(countIndex);
    
    /*Serial.print("dial ");
    Serial.print(i);
    Serial.print(" answer = ");
    Serial.println(answer);
    */
  }
}

void parseSliders(){
  int dataIndex = 11; //pointer to starting index in dataInstance
  for(int i=0; i<5; i++){
    int answer = dataInstance[dataIndex++] - '0';
    int countIndex;
    if(answer<2){
      countIndex = pgm_read_byte(&(sliderAnswers[(i*6)]));
    }else if(answer>=2 && answer<4){
      countIndex = pgm_read_byte(&(sliderAnswers[(i*6)+1]));
    }else if(answer==4){
      countIndex = pgm_read_byte(&(sliderAnswers[(i*6)+2]));
    }else if(answer == 5){
      countIndex = pgm_read_byte(&(sliderAnswers[(i*6)+3]));
    }else if(answer>=6 && answer<8){
      countIndex = pgm_read_byte(&(sliderAnswers[(i*6)+4]));
    }else{
      countIndex = pgm_read_byte(&(sliderAnswers[(i*6)+5]));
    }
    int tmp = answerCount[countIndex];
    tmp = tmp+2;
    answerCount[countIndex] = tmp;

    //debugging
    //Serial.print("slider countIndex = ");
    //Serial.println(countIndex);
    
    /*Serial.print("slider ");
    Serial.print(i);
    Serial.print(" answer = ");
    Serial.println(answer);*/
  }
}

void parseSpinners(){
  int dataIndex = 16; //pointer to starting index in dataInstance
  for(int i=0; i<2; i++){
    int answer = dataInstance[dataIndex++] - '0';
    int countIndex = pgm_read_byte(&(spinnerAnswers[(i*5)+answer]));
    int tmp = answerCount[countIndex];
    tmp = tmp+1;
    answerCount[countIndex] = tmp;
    
    //debugging
    //Serial.print("spinner countIndex = ");
    //Serial.println(countIndex);
    
    /*Serial.print("spinner ");
    Serial.print(i);
    Serial.print(" answer = ");
    Serial.println(answer);*/
  }
}

void printResult(int maxIndex){  
  Serial.println("printing results...");
  
  //print elements in reverse for upside down receipt
  Printer.write(10);
  Printer.write(10);
  printURL();
  
  Printer.write(10);
  Printer.write(10);
  printFooter(maxIndex);
  
  Printer.write(10);
  Printer.write(10);
  
  //print icon
  if(maxIndex == 0){
    printMechanical();
  }else if(maxIndex == 1){
    printElectrical();
  }else if(maxIndex == 2){
    printAerospace();
  }else if(maxIndex == 3){
    printBiochemical();
  }else if(maxIndex == 4){
    printSoftware();
  }else{
    printCivil();
  }
  
  Printer.write(10);
  Printer.write(10);
  
  printSubtext();
  Printer.write(10);
  
  printHeader();
  Printer.write(10);
  Printer.write(10);
  Printer.write(10);
  
  printCutter();
  Printer.write(10);
  Printer.write(10);
  Printer.write(10);
  Printer.write(10);
}


void printCutter(){
  Printer.printBitmap(384,42,papercut_data);
}

void printHeader(){
  char nextChar;
  int len = strlen_P(header);
  for (int i=0; i < len; i++)
  {
    nextChar =  pgm_read_byte_near(header + i);
    Printer.print(nextChar);
  }
  Printer.println();
}

void printSubtext(){
  char nextChar;
  int len = strlen_P(subtext);
  for (int i=0; i < len; i++)
  {
    nextChar =  pgm_read_byte_near(subtext + i);
    Printer.print(nextChar);
  }
  Printer.println();
}

void printFooter(int maxIndex){
  strcpy_P(buffer, (char*)pgm_read_word(&(engineers[maxIndex])));
  Printer.justify('C');
  
  char nextChar;
  
  //Print end of answer
  int len = strlen_P(answer_end);
  for (int i=0; i < len; i++)
  {
    nextChar =  pgm_read_byte_near(answer_end + i);
    Printer.print(nextChar);
  }
  Printer.println();
  Printer.println(buffer);
  Printer.println();
  Printer.println();
  Printer.justify('L');
  
  //print start of answer
  len = strlen_P(answer_start);
  for (int i=0; i < len; i++)
  {
    nextChar =  pgm_read_byte_near(answer_start + i);
    Printer.print(nextChar);
  }
  Printer.println();
}

void printURL(){
  Printer.setSize('S');
  char nextChar;
  int len = strlen_P(url);
  for (int i=0; i < len; i++)
  {
    nextChar =  pgm_read_byte_near(url + i);
    Printer.print(nextChar);
  }
  Printer.println();
  Printer.setSize('L');
}

void printMechanical(){
  Printer.printBitmap(80,80,gear_data);
}

void printElectrical(){
  Printer.printBitmap(80,127,electric_data);
}

void printAerospace(){
  Printer.printBitmap(80,80,rocket_data);
}

void printBiochemical(){
  Printer.printBitmap(80,88,beaker_data);
}

void printSoftware(){
  Printer.printBitmap(80,66,computer_data);
}

void printCivil(){
  Printer.printBitmap(80,73,crane_data);
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
    
    //Serial.print("control message to servoId: ");
    //Serial.println(servoId);
    
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
  setDefaultSlaves();
}

boolean connectToWifi(){
  int tries = 0;
  while(status != WL_CONNECTED && tries < 10){
    
    Serial.print("Connecting to wifi. Attempt ");
    Serial.print(tries+1);
    Serial.println("...");
    
    status = WiFi.begin(ssid/*, password*/);
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
