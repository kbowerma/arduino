/*Kyle Bowerman
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

Derived from sensorTestF_ver3 for Mega with ethersheild on 6/26/2011
8/1 adding rtc support
  this code has a bug the webserver dies and the serial port becomes un responsive
  moved showtime(), and read temp to function 
    Left:  this is broken the webserver kills it   I moved on to Mega_dtemp_bracnh_Case
        sending w starts the webserver
        #mystrey I could not read the temp sensors so I commeted everything out then added it back and it works now. 
*/


#include <Ethernet.h>
#include <EEPROM.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
//#include <WProgram.h>
#include <Wire.h>
#include <DS1307.h>



// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 53

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192, 168, 11, 82 };
Server server(80);




// ------------Begin  Functions ---------------------


void cleareeprom(int first, int last){
  for(int j=first; j<=last; j++){
    EEPROM.write(j,0);
  }
}  


  
void showtime(){
      Serial.print(RTC.get(DS1307_HR,true)); //read the hour and also update all the values by pushing in true
      Serial.print(":");
      Serial.print(RTC.get(DS1307_MIN,false));//read minutes without update (false)
      Serial.print(":");
      Serial.print(RTC.get(DS1307_SEC,false));//read seconds
      Serial.print("	");	// some space for a more happy life
      Serial.print(RTC.get(DS1307_MTH,false));//read date
      Serial.print("/");
      Serial.print(RTC.get(DS1307_DATE,false));//read month
      Serial.print("/");
      Serial.print(RTC.get(DS1307_YR,false)); //read year
      Serial.println();
   }  
  
  
  
void readtemp(byte i, byte present, byte data[12], byte addr[8], int devices){
 // int devices =  sensors.getDeviceCount();   //count the 1wire devices

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

void debug(byte i, byte present, byte data[12], byte addr[8], int devices){
    
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


void startwebserver(){
  Serial.println("Trying to start webserver ....");
  
  Client client = server.available();
  Serial.println("Still Trying to start webserver ....");
  if (client) {
    Serial.println("starting webserver");
    // an http request ends with a blank line
    boolean current_line_is_blank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // if we've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so we can send a reply
        if (c == '\n' && current_line_is_blank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();

          // output the value of each analog input pin
 
            client.print("<brHello world");
            
  
          break;
        }
       
     
        if (c == '\n') {
          // we're starting a new line
          current_line_is_blank = true;
        } else if (c != '\r') {
          // we've gotten a character on the current line
          current_line_is_blank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    client.stop();
  }
  else {
   Serial.println("can't start webserver");
  }
 
}  // end startwebserver
 
 
//---------------- end functions ---------------


void setup()
{
  Serial.begin(9600);
//  Ethernet.begin(mac, ip);
 // server.begin();
  Serial.println("Dallas Temperature IC Control Library Demo");
  Serial.println("File: MegaEthtemp_v2");
  Serial.println("Ip address:  192.168.11.82");
  Serial.println("Features: RTC, dtemp-wrong, webserver(broken)");
 
 // Start up the library
  sensors.begin();
  
  
        
        

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
  int devices =  sensors.getDeviceCount();
  Serial.println(devices);
  Serial.println(" devices");
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  
  byte i;                           //onewire debug code
  byte present = 0;                 //onewire debug code
  byte data[12];                    //onewire debug code
  byte addr[8];                     //onewire debug code
  
   
 
  
  
      
  //----- start relay on off.
        if (Serial.available() > 0) {
          
          int inByte = Serial.read();
          
          switch (inByte) {
           //On      
            case '1':
             digitalWrite(10, HIGH);
             Serial.println("Turning Buzzer on");
            break;
           //off 
            case '0':
             digitalWrite(10,LOW);
             Serial.println("turing Buzzer off");
            break;
             
            case 'r':
              readtemp(i,present,data,addr, devices);
            break;
            
            case 'x':
            debug(i,present,data,addr, devices);  
            break;
            
            case 'c':
           showtime();
            break;
          
            case 'w':
          startwebserver();
            break;
           default:
             digitalWrite(3,LOW);  
          }  
        }
    
  
  delay(1000);
  
  //  end digital temp code from digitaltemp1_light_buzzer
 
   
  
      
   
   // begin Serial debug

   Serial.println( millis()/1000);


 }  //end loop



