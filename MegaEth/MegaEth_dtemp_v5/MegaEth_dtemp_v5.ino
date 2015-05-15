/*
Kyle Bowerman
last updated 12.26.2012
http://www.ladyada.net/make/logshield/lighttempwalkthru.html

*/


#include <Streaming.h>
#include <SD.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
//#include <util.h>
#include <EEPROM.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "RTClib.h"
//#include <stdlib.h>

RTC_DS1307 RTC;   // define the Real Time Clock object


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
// A simple data logger for the Arduino analog pins
#define LOG_INTERVAL  1000 // mills between entries
#define ECHO_TO_SERIAL   1 // echo data to serial port
#define WAIT_TO_START    0 // Wait for serial input in setup()

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192,168,11,82 };
byte subnet[] = {255,255,255,0};
EthernetServer server(80);

// the logging file
File logfile;

void error(char *str) {

  Serial.print("error: ");
  Serial.println(str);
  
  // red LED indicates error
  //digitalWrite(redLEDpin, HIGH);
  
  while(1);
}


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
void showtime(){
	DateTime now;
  // fetch the time

  now = RTC.now();
  // log time
  dataFile.print(now.get()); // seconds since 2000
  dataFile.print(", ");
  dataFile.print(now.year(), DEC);
  dataFile.print("/");
  dataFile.print(now.month(), DEC);
  dataFile.print("/");
  dataFile.print(now.day(), DEC);
  dataFile.print(" ");
  dataFile.print(now.hour(), DEC);
  dataFile.print(":");
  dataFile.print(now.minute(), DEC);
  dataFile.print(":");
  dataFile.print(now.second(), DEC);
#if ECHO_TO_SERIAL
  Serial.print(now.get()); // seconds since 2000
  Serial.print(", ");
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
#endif //ECHO_TO_SERIAL
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
 #if WAIT_TO_START
  Serial.println("Type any character to start");
  while (!Serial.available());
 #endif //WAIT_TO_START
   
  // Start up the onewire library
  sensors.begin();
  Ethernet.begin(mac, ip);
  server.begin();
 // Wire.begin();   //added 12/16 to debug showdate() format issue
 // RTC.begin();
  
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
   
      Wire.begin();  
	  if (!RTC.begin()) {
	    datafile.println("RTC failed");
	#if ECHO_TO_SERIAL
	    Serial.println("RTC failed");
	#endif  //ECHO_TO_SERIAL
	  }   

// Clear the eeprom
  for(int k=1; k<=255; k++){  // clear the EEPROM
    EEPROM.write(k,0);
  }
//  delay(1000);

}

void loop()
{
  
 // DateTime now;  // moved to function

  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));

  // log milliseconds since starting
  uint32_t m = millis();
  datafile.print(m);           // milliseconds since start
  datafile.print(", ");    
#if ECHO_TO_SERIAL
  Serial.print(m);         // milliseconds since start
  Serial.print(", ");  
#endif
  
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
	

//----- datalogger example: http://arduino.cc/en/Tutorial/Datalogger
if (millis()/1000 % 60  == 0) {   // 60 for once a minutes
// make a string for assembling the data to log:
  String dataString = "";


 writecount++;
 for ( int c = 0; c < 2; c++) {
  showtime();
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
