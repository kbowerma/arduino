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
  8/1 cant hit the webserver
*/


#include <Ethernet.h>
#include <EEPROM.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>



// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 43

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xDD };
byte ip[] = { 192, 168, 11, 82 };
Server server(80);




// ------------Begin  Functions ---------------------


void cleareeprom(int first, int last){
  for(int i=first; i<=last; i++){
    EEPROM.write(i,0);
  }
  

}
  
//---------------- end functions ---------------


void setup()
{
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  Serial.println("Dallas Temperature IC Control Library Demo");
  Serial.println("File: MegaEthtemp_v1");
  Serial.println("Ip address:  192.168.11.82");
  server.begin();
 // Start up the library
  sensors.begin();

// Clear the eeprom
  for(int i=1; i<=255; i++){  // clear the EEPROM
    EEPROM.write(i,0);
  }
//  delay(1000);

}

void loop()
{
  // start digital temp code from digitaltemp1_light_buzzer
  //count the 1wire devices
  int devices =  sensors.getDeviceCount();
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
        Serial.print(" Requesting temperatures...");
        sensors.requestTemperatures(); // Send the command to get temperatures
        Serial.println("DONE");
        
        Serial.print("Devices found: ");
        Serial.println(devices);
        Serial.print("Temperature for Device 1 is: ");
        Serial.println(sensors.getTempFByIndex(0)); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
        
        Serial.print("Temperature for Device 2 is: ");
        Serial.println(sensors.getTempFByIndex(1)); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire

      break;
      
      case 'x':
      Serial.println("");
      Serial.println("first Pass");
      for ( i = 0; i < 9; i++) {           // we need 9 bytes
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
      
      break;
      
      
      
      default:
       digitalWrite(3,LOW);  
    }  
  }
  
  delay(2000);
  
  //  end digital temp code from digitaltemp1_light_buzzer
 
   
  
      
   
   // begin Serial debug
   Serial.println("");
   Serial.println("file: MegaEthtemp_v1.pde");

      
  
  
 //-------------------- start webserver ---------- 
  Client client = server.available();
  if (client) {
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
 
            client.print("<br>6/22/2010 file: MegaEthtemp_v1.pde<br> time to next EEPROM write: ");
            client.print("Serial Available ");
            client.print(Serial.available());

            client.print("<table><th>Sensor1</th><th>Sensor 2</th><tr><td>");
            for (int addr = 1; addr <= 50; addr++){
              //client.print(x);
              //client.print(" "); client.print(millis() % 10 );  // for dubuggin
              client.print(" EEPROM ");
              client.print(addr);
              client.print("= ");
              int myeepromvalue = EEPROM.read(addr);
              client.print(myeepromvalue);
              if (myeepromvalue != 0) {
              client.print(" temp ");
    
              }
              client.print("<br>");
              if (addr % 10 == 0){client.print("</tvd><td>");}   
            }
            client.print("</table>");
            
        client.print("Devices found: ");
        client.println(devices);
        client.print("Temperature for Device 1 is: ");
        client.println(sensors.getTempFByIndex(0)); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
        
        client.print("Temperature for Device 2 is: ");
        client.println(sensors.getTempFByIndex(1)); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
        
            
           /*
            for (int z = 1; z < 301; z++){
              client.print(sensor1Array[z]);
              client.print(" ");
            }
          */

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
 }



