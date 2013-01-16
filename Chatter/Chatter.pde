/* 
 * //////////////////////////////////////////////////
 * //Sensor Chatter for Force.com
 * //////////////////////////////////////////////////
 *
 * Detects motion and light changes and updates Force.com Internet web service
 *
 * @author: Michael A Leach mike@cubiccompass.com
 * @date:   May 2010
 * (cleft) 2010
 * Released under a Creative Commons Attribution-Noncommercial-Share Alike 3.0 United States License
 * http://creativecommons.org/licenses/
 *
 * The Parallax PIR Sensor is an easy to use digital infrared motion sensor module. 
 * (http://www.parallax.com/detail.asp?product_id=555-28027)
 *  
 */

#include <Ethernet.h>
 
// a unique MAC address to identify the Arduino
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// this is the IP that will identify the Arduino on the Chatter subnet
byte ip[] = { 10, 1, 10, 150 };
byte chatterProxyService[] = { 10, 1, 10, 30 };  //proxy service for communicating with Salesforce
int PROXY_SERVICE_PORT  = 80;

/////////////////////////////
//VARS
//the time when the sensor outputs a low impulse
long unsigned int lowIn;

//the amount of milliseconds the sensor has to be low 
//before we assume all motion has stopped
long unsigned int pause = 5000;  

boolean lockLow = true;
boolean takeLowTime;  

int PIR_PIN = 3;    // the digital pin connected to the PIR sensor's output
int LDR_PIN = 2;    // the analog pin for reading the LDR (Light Dependent Resistor)
int LED_PIN = 13;

byte LIGHT_ON    = 1;
byte LIGHT_OFF   = 0;
byte previousLightState  = LIGHT_ON;
int lightLastChangeTimestamp = 0;
unsigned int LIGHT_ON_MINIMUM_THRESHOLD = 1015;
unsigned long lastListStateChange = 0; //Used to de-bounce borderline transitions.

// Messages
int MESSAGE_TYPE_INIT              = 1;
int MESSAGE_TYPE_MOTION_DETECTED   = 2;
int MESSAGE_TYPE_LIGHT_ON          = 3;
int MESSAGE_TYPE_LIGHT_OFF         = 4;

Client client(chatterProxyService, PROXY_SERVICE_PORT);

/////////////////////////////
//SETUP
void setup(){  
  //PIR initialization
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(PIR_PIN, LOW);
  
  Ethernet.begin(mac, ip);
  Serial.begin(9600);
  
  InitializeChatterConnection();
  InitializeMotionSensor();
}

////////////////////////////
//LOOP
void loop(){
  
  if(digitalRead(PIR_PIN) == HIGH){
    digitalWrite(LED_PIN, HIGH);   //the led visualizes the sensors output pin state
    if(lockLow){
      //makes sure we wait for a transition to LOW before any further output is made:
      lockLow = false;
      Serial.println("---");
      Serial.print("motion detected at ");
      Serial.print(millis()/1000);
      Serial.println(" sec");
      
      UpdateChatterService(MESSAGE_TYPE_MOTION_DETECTED);
      
      delay(50);
      digitalWrite(LED_PIN, LOW);   //the led visualizes the sensors output pin state
    }
    takeLowTime = true;
  }

  if(digitalRead(PIR_PIN) == LOW){
    digitalWrite(LED_PIN, LOW);  //the led visualizes the sensors output pin state
    if(takeLowTime){
      lowIn = millis();          //save the time of the transition from high to LOW
      takeLowTime = false;       //make sure this is only done at the start of a LOW phase
    }
    
    //if the sensor is low for more than the given pause, 
    //we assume that no more motion is going to happen
    if(!lockLow && millis() - lowIn > pause){
      //makes sure this block of code is only executed again after 
      //a new motion sequence has been detected
      lockLow = true;
      Serial.print("motion ended at ");      //output
      Serial.print((millis() - pause)/1000);
      Serial.println(" sec");
            
      delay(50);
    }
  }
  
  ProcessLightSensor();
}

boolean InitializeChatterConnection(){ 
  delay(1000);
  Serial.print("connecting to chatter proxy service...");
  
  UpdateChatterService(MESSAGE_TYPE_INIT);
}

void UpdateChatterService(int msgType){
  if (client.connect()) {
    Serial.print(" - Connected");
    
    // Send the HTTP GET to the server
    switch(msgType){
       case 1: 
         Serial.println("Sending init feed post");
         client.println("GET /proxy.ashx?m=Motion%20sensor%20initializing HTTP/1.1");
         break;
       case 2:
         Serial.println("Sending motion detected feed post");
         client.println("GET /proxy.ashx?m=Motion%20detected HTTP/1.1");
         break;
       case 3:
         Serial.println("Lights turned on");
         client.println("GET /proxy.ashx?m=Lights%20on HTTP/1.1");
         break;
       case 4:
         Serial.println("Lights turned off");
         client.println("GET /proxy.ashx?m=Lights%20off HTTP/1.1");
         break;
       default:
         client.println("GET /proxy.ashx HTTP/1.1");
         break;
    }
    client.println("Host: 10.1.10.30");
    client.println("User-Agent: Mozilla/5.0 (Arduino)");
    client.println("Accept: text/plain,text/html,application/xhtml+xml,application/xml");
    client.println("Accept-Language: en-us,en");
    client.println("Accept-Charset: ISO-8859-1,utf-8");
    client.println("Keep-Alive: 300");
    client.println("Connection: keep-alive");
    client.println();

    // Read the response
    Serial.println(" - ");
    Serial.print(ReadResponse(), DEC);
    Serial.println(" bytes received");
    
    Serial.println(" __________________________________________________ ");
    Serial.println("");
    // Disconnect from the server
    client.flush();
    client.stop();
    Ethernet.begin(mac, ip);
  }
  else {
    // Connection failed
    Serial.println(" - CONNECTION FAILED!");
    client.flush();
    client.stop();
  }
}

//the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int calibrationTime = 30;

boolean InitializeMotionSensor(){
  //give the sensor some time to calibrate
  Serial.print("calibrating sensor ");
  for(int i = 0; i < calibrationTime; i++){
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" done");
  Serial.println("SENSOR ACTIVE");
  delay(50);
}

int ReadResponse(){
  int totalBytes=0;
  unsigned long startTime = millis();

  // First wait up to 5 seconds for the server to return some data.
  while ((!client.available()) && ((millis() - startTime ) < 5000));

  while (client.available()) {
    char c = client.read();
    Serial.print(c);
    totalBytes+=1;
  }
  Serial.println("");
  Serial.println("");
  return totalBytes;
}

boolean ProcessLightSensor(){
  byte currentState = previousLightState;
  int lightLevelValue = analogRead(LDR_PIN);  // returns value 0-1023. 0=max light. 1,023 means no light detected.
  
  if(lightLevelValue < LIGHT_ON_MINIMUM_THRESHOLD){
     currentState = LIGHT_ON;
  }
  else{
     currentState = LIGHT_OFF;
  }
  
  if(LightStateHasChanged(currentState) && !LightStateIsBouncing() ){
    previousLightState = currentState; 
    
    if(currentState == LIGHT_ON){
      Serial.println("------------------- Lights on");
      UpdateChatterService(MESSAGE_TYPE_LIGHT_ON);
    }
    else{
      Serial.println("------------------- Lights off");
      UpdateChatterService(MESSAGE_TYPE_LIGHT_OFF);
    }
    
    delay(2000);
    lightLastChangeTimestamp = millis();
    
    return true;
  }
  else{
    return false; 
  }
}

boolean LightStateHasChanged(byte currentState){
   return currentState != previousLightState; 
}

//De-bounce LDR readings in case light switch is being quickly turned on/off
unsigned int MIN_TIME_BETWEEN_LIGHT_CHANGES = 5000;
boolean LightStateIsBouncing(){
   if(millis() - lightLastChangeTimestamp < MIN_TIME_BETWEEN_LIGHT_CHANGES){
      return true; 
   }
   else{
      return false; 
   }
}
