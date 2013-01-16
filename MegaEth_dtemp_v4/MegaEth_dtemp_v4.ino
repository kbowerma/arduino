/*
Kyle Bowerman
last updated 6.15.2011
function:  webserver provides temprature reads of 6 analog temp sensors
and provides some debug via serial
and loads values into EEPROM

next steps: make some unputs that clear something, I have a listner 
add a case to determing the lighting
449 Halogen on
885 room light on
834 day light
989 room light off door open
1016 night and door closed

from datalogger http://arduino.cc/en/Tutorial/Datalogger
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
also pin 10 is used for output

Derived from sensorTestF_ver3 for Mega with ethersheild on 6/26/2011
8/1 adding rtc support
  this code has a bug the webserver dies and the serial port becomes un responsive
  moved showtime(), and read temp to function 
    Left:  this is broken the webserver kills it   I moved on to Mega_dtemp_bracnh_Case
        sending w starts the webserver
        #mystrey I could not read the temp sensors so I commeted everything out then added it back and it works now. 
          solved: starting ethernet seems to kill the dtep.   also does reading the sd card
          solved moved the dtep pin to 43 now it works with ethernet.   Ethernet bust be pin 53 will try the webserver now
        
        
        #lastUpdate
          I show the time but when I hit the webserver too many times it stops responding but
          if I open or close the serial port it seems to free it.   I am going to try to stop
          writing to the seral and just use my case to show some debug logs.  I can only hit it one time
 8/2  I can hit it more now but lastnight it would lock up after a few hits.
          but now it seems like the sensor temp is stuck it should be 74 but it is reading 78
*/


#include <Streaming.h>
#include <SD.h>
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <util.h>
#include <EEPROM.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
//#include <DS1307.h>   // I had to comment this out becuse of line RTC_DS1307 RTC;
#include "RTClib.h"
#include <stdlib.h>

RTC_DS1307 RTC;

// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.
const int chipSelect = 4;

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 43
int pin1 = 34;
int pin2 = 36;
int pin3 = 38;
int pin4 = 40;
int ledpin[4] = {34, 36, 38, 40};
byte ledcode = 0;
int writecount = 0;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192,168,11,82 };
byte subnet[] = {255,255,255,0};
EthernetServer server(80);


// ------------Begin  Functions ---------------------

void deletefile(){
   Serial.println("deleting file ...");  // the << does not seem to wrok anymore
   SD.remove("datalog.txt");
}
void dumpfile(){
 // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt");

  // if the file is available, write to it:
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }	
}
void cleareeprom(int first, int last){
  for(int j=first; j<=last; j++){
    EEPROM.write(j,0);
  }
}  
String showtime(){
   // char time =RTC.get(DS1307_HR,true);
    DateTime now = RTC.now();
  
    String myYear = String(now.year());
    String myMonth = String(now.month());
    String myDay = String(now.day());
    String myHour = String(now.hour());
    
    
    
    String myMinute = String(now.minute());
    if (now.minute() < 10) { 
      myMinute = String("0"+ String(now.minute()));
    }
    
    String mySecond = String(now.second());
    if (now.second() < 10) {
     mySecond = String("0"+ String(now.second()));
    } 
    
    String myDate = String(myMonth+"/"+myDay+"/"+myYear);
    String myTime = String(" "+myHour+":"+myMinute+":"+mySecond);
    
    String myDateTime = String(myDate + myTime);

Serial << endl << "myDateTime from function: " << myDateTime << endl;
    
    return myDateTime;
}  
float getTemp(int index){ 
	float mystring = 0;  // put this in to see if the fail is due to requestTemp call
	sensors.requestTemperatures();  // Send the command to get temperatures
   mystring = sensors.getTempFByIndex(index);
   return mystring;
}
String getTempString(int index){
 char buffer[16];
 //String mystring = '';
 String mystring = dtostrf(getTemp(index), 2, 2, buffer);
 return mystring;
}
void readtemp(byte i, byte present, byte data[12], byte addr[8]){
  int devices =  sensors.getDeviceCount();   //count the 1wire devices

  Serial.print(" Requesting temperatures...");
        sensors.requestTemperatures(); // Send the command to get temperatures
        Serial.println("DONE");
        
        Serial.print("Devices found: ");
        Serial.println(devices);
        Serial.print("Temperature for Device 1 is: ");
        Serial.println(sensors.getTempFByIndex(0)); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
        
        Serial.print("Temperature for Device 2 is: ");
        Serial.println(sensors.getTempFByIndex(1)); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire

}
void debug(byte i, byte present, byte data[12], byte addr[8]){
    
      Serial.println("");
      Serial.println("first Pass");
      for (int i = 0; i < 9; i++) {           // we need 9 bytes
       Serial.print(addr[i],HEX);
       Serial.print(" ");
      }
      Serial.println("");
      Serial.println("second pass:");
      oneWire.search(addr);
      for ( i = 0; i < 9; i++) {           // we need 9 bytes
       Serial.print(addr[i],HEX);
       Serial.print(" ");
      }
      Serial.println("");
}
void mydisplay(byte code){
  //	Serial << "the byte for " <<code << " is " ; //  took out Mikal Harst Streaming 4
      Serial << "the byte for " <<code << " is " ;
	for (int n = 7; n > -1; n--){
		Serial << bitRead(code,n);
	}
	Serial << endl;
}
void setled(byte code){
	int blink = bitRead(code,7);
		for (int i = 1; i<5; i++){
			int mylead = bitRead(code, i-1);
			 if (mylead == 1){
				digitalWrite(ledpin[i-1],HIGH);
			}
				else {digitalWrite(ledpin[i-1],LOW); }
				
			
		}
		
	
}
void displayMenu(){
	Serial << endl << "Serial Input menu:" << endl;
	Serial << " r = read temp " << endl;
	Serial << " x = debug: " << endl;
	Serial << " c = show time " << endl;
	Serial << " u = dump file " << endl;
	Serial << " d = delete file " << endl;
	Serial << " 0 = setled value 0 " << endl;
	Serial << " 1 = setled value 15 " << endl;
	Serial << " 3 = setled value 3 " << endl;
	Serial << " z = clear leds " << endl;
	
}
//---------------- end functions ---------------


void setup()
{
  Serial.begin(9600);
   
  // Start up the onewire library
  sensors.begin();
  Ethernet.begin(mac, ip);
  server.begin();
  Wire.begin();   //added 12/16 to debug showdate() format issue
  RTC.begin();
  
  Serial << "File: MegaEthtemp_v3 " << "Ip address: 192.168.11.82" << endl; 
  Serial << "Last Update 12/23/2012 " << endl; 
  Serial <<  "Features: RTC, dtemp, webserver(broken), SD" << endl;
Serial << "type ? for menu help " << endl;
  int devices =  sensors.getDeviceCount();
  Serial << "devices found: " << devices << endl;
 // ----- Sd Card
  Serial << "Initializing SD card...";
//  // make sure that the default chip select pin is set to
//  // output, even if you don't use it:
  pinMode(53, OUTPUT);
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  pinMode(pin4, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
	setled(5);
    Serial << "Card failed, or not present";
    // don't do anything more:
    return;
  }
  Serial << "card initialized." << endl;
  setled(1);
Serial << " return from setled call" << endl;
 
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {
//	dataFile.print(showtime());
//	dataFile.print("  Rebooted ");
	setled(3); // debug that the sd card is present
	delay(2000);
	setled(0);
	dataFile << showtime() << " Rebooted" << endl;
    dataFile.close();
    Serial.println("Reboot"); // print to the serial port too:
  }  
  else {
	for (int i=0;i<10;i++){
	 setled(5);
	 delay(100);
	 setled(0);
	 delay(100);
    }
    Serial.println("error opening datalog.txt"); // if the file isn't open, pop up an error:
    
  }
   
        

// Clear the eeprom
  for(int k=1; k<=255; k++){  // clear the EEPROM
    EEPROM.write(k,0);
  }
//  delay(1000);

}

void loop()
{
  
  // start digital temp code from digitaltemp1_light_buzzer
  //count the 1wire devices
  //int devices =  sensors.getDeviceCount();  // try moving this to startup
  //Serial.println(devices);
  //Serial.println(" devices");
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  
  byte i;                           //onewire debug code
  byte present = 0;                 //onewire debug code
  byte data[12];                    //onewire debug code
  byte addr[8];                     //onewire debug code
      
 
        if (Serial.available() > 0) {
          
          int inByte = Serial.read();
          
          switch (inByte) {
            case 'r':
              readtemp(i,present,data,addr);
            break;
            
            case 'x':
            debug(i,present,data,addr);  
            break;
            
            case 'c':
           Serial << showtime();
		 for (int x = 1; x < 50; x++){
			digitalWrite(13,LOW); 
			delay(200);
			digitalWrite(13,HIGH); 
			delay(200);
		}
			digitalWrite(13,LOW); 
            break;
          
            case 'u':
			dumpfile();
            break;

			case '?':
			displayMenu();
			break;

            case 'd':  // delete file
            deletefile();
            // Check to see if the file exists: 
	         if (SD.exists("datalog.txt")) {
	          Serial.println("datalog.txt exists.");
	     }
	        else {
	         Serial.println("datalog.txt doesn't exist.");  
	        }
           break;

	      case '0':
		   setled(0);
		    break;
		
		  case '1':
		   setled(15);
		    break;
		  case '3':
			setled(3);
		  break;
		
		 case 'z':
		digitalWrite(34,LOW);
		digitalWrite(36,LOW);
		digitalWrite(38,LOW);
		digitalWrite(40,LOW);
		Serial << "Tried to clear LEDs";
		break;
		
           default:
             digitalWrite(13,LOW);  
          }  
        }
	//	else { setled(4);}
	
	
	// debug show time
//	Serial << endl << "myDateTime: " << showtime() << endl;  // this gives 165/165/165 165:165:85
	
	

//----- datalogger example: http://arduino.cc/en/Tutorial/Datalogger
if (millis()/1000 % 60  == 0) {   // 60 for once a minutes
// make a string for assembling the data to log:
  String dataString = "";

  // read three sensors and append to the string:
 /* for (int analogPin = 0; analogPin < 3; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 2) {
      dataString += ","; 
    }
  }

*/
 writecount++;
 for ( int c = 0; c < 2; c++) {
  dataString = showtime();
  dataString += "     device: ";
  dataString += c;
  dataString += "  ";
  dataString += getTempString(c);
  dataString += "  WriteCounter: ";
  dataString += writecount;


  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
/*	if(writecount/4 ==0){
		setled(0);
	} else {
		setled(1);
	}*/
		setled(writecount % 16);
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
 }
  } 
}

//------ end datalogger example
  
 
   
  
      
   
   // begin Serial debug

  // Serial.println( millis()/1000);
/*
//----------- webserver 
 //Serial.println("waiting for incoming requests");
  Client client = server.available();
  //Serial.println("client is available");
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();

          // output the value of each analog input pin
      for (int index = 0; index < 2; index++){
          client.print(showtime());
          client.print("  ");
          client.print(getTemp(index));
           client.print("<br>  ");
      }
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(10);
    // close the connection:
    client.stop();
  } 
*/
}
