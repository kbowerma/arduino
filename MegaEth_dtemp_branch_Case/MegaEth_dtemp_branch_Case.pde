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
 8/1   this is to merge MegaEthdtemp_v2 onto MegaEthdtemp+branch_case
   TODO:
     fix so I am using onwire not anolog temp
     done: write the time as a string
     
 Ever since I add the time call to the web server it does not respond after the first eeprom write
 since I added the rtc I don't seem to be able to read the one wire bus
 
 
     
*/

/*
#include <Ethernet.h>
#include <EEPROM.h>
#include <SPI.h>
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
char mychar = 'A';
int mylight = 0;
int mylightvalue;
String stringone = "init-0";


Server server(80);

float myk[] = {5.005, 5.005};
const int interval1 = 30;
float sensor1Array[300]; 
//int c2 = 1;
//int c1 = 1;

// ------------Begin  Functions ---------------------
const float get_temperature(int mypin, float k) {
  //Serial.print("|");
  const int sensor_value = analogRead(mypin);
  const float sensor_voltage = sensor_value * k / 1024;
  const float tempc = (sensor_voltage - 0.5 ) * 100;
  const float tempf = (tempc * 9 / 5) + 32;
  const float mytempf = ((((sensor_value)*k/1024)-0.5)*180)+32;
  return mytempf;
}

 void myoutput(int mypin, float k ){
   const int sensor_value = analogRead(mypin);
   const float sensor_voltage = sensor_value * k / 1024;
  Serial.print(millis()/1000);Serial.print("|");
  Serial.print(mypin);Serial.print("|");
  Serial.print(sensor_value);Serial.print("|");
  Serial.print(sensor_voltage);Serial.print("|");
  Serial.print(get_temperature(mypin,k));Serial.println("|");
  

}

const float convert2f(int value, int pin){
  float temp = (((value)*myk[pin]/1024)-0.5)*180+32;
  return temp;
}

void cleareeprom(int first, int last){
  for(int i=first; i<=last; i++){
    EEPROM.write(i,0);
  }
}

String showtime(){
    
      String time =RTC.get(DS1307_HR,true);
      time +=":";
      time +=RTC.get(DS1307_MIN,true);
      time +=":";
      time +=RTC.get(DS1307_SEC,true);
      time +=" ";
      time +=RTC.get(DS1307_MTH,true);
      time +="/";
      time +=RTC.get(DS1307_DATE,true);
      time +="/";
      time +=RTC.get(DS1307_YR,true);
      Serial.println(time);
      return time;  
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
  
//---------------- end functions ---------------


void setup()
{
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  server.begin();
 // sensors.begin();

  for(int i=1; i<=255; i++){  // clear the EEPROM
    EEPROM.write(i,0);
  }
  delay(10000);
 // int c = 1; // counter for interval writing to eeprom
 // int c2 = 0;  // counter for 50 minute aggregate
}

void loop()
{
  int devices =  sensors.getDeviceCount();
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  
  byte i;                           //onewire debug code
  byte present = 0;                 //onewire debug code
  byte data[12];                    //onewire debug code
  byte addr[8];                     //onewire debug code
  
      int c1 = 1; 
      int c2 = 1;
      while ( c1 <= 10 ) {
    //  int sensorPin1 = 0;
     // int sensorPin2 = 1;
      int sec = millis() / 1000;
      int x = millis()/1000 % interval1;
      if ( x == 0 ) {  
      EEPROM.write(c1,analogRead(0));
      EEPROM.write(c1+10,analogRead(1));
      if (c1 == 10) {                            // after 10 fill up find the average
        int sum1 = 0;
        int sum2 = 0;
        for ( int y = 1; y <= 10; y++ ){
          sum1 += EEPROM.read(y);
          sum2 += EEPROM.read(y+10);
        }
          EEPROM.write(c1+20+c2,sum1/10);
          EEPROM.write(c1+30+c2,sum2/10);
          //call clear eeprom 1-20
          cleareeprom(1,20);
          if ( c2 == 10 ) {
            c2 = 0;
          } else {
              c2 += 1;
            }
      }  
      mylightvalue = analogRead(2);
     if (mylightvalue > 400) mylight = 1;
     if (mylightvalue > 800){
	 mylight = 2;
         stringone = "room light or daylight";
     }
     if (mylightvalue > 900) mylight = 3;
     if (mylightvalue > 1000) mylight = 4;
   
   // begin Serial debug
   Serial.println("");
   Serial.println("file:MegaEth_dtemp_branch_Case");
   int i = 0;
  while (i < 6) {
  myoutput(i, myk[i]);
  i++;
  }
   // end serial debug
      delay(1500);
      c1++;
      
   
   //begin Serial input
   // send data only when you receive data:
	if (Serial.available() > 0) {
		// read the incoming byte:
		int mychar = Serial.read();

		// say what you got:
                   switch (mychar){
                     case 'a':
    		     Serial.print("I received a ");
    		     Serial.println(mychar);
                     break;
                     case 'r':
    		       readtemp(i,present,data,addr);
                     break;
                     case 'c':
                     showtime();
                     break;
                     default:
                     Serial.println("I did not recieve a or b");
                     
	}}
 
 
 
 }
  
  
  
  
  
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
            client.print("Milliseconds since boot: ");
            client.print(millis());
            client.print("<br> Seconds since boot: ");
            client.print(sec);
            client.print("<br>6/22/2010 file: MegaEth_dtemp_branch_Case.pde<br> time to next EEPROM write: ");
     //       client.println(showtime());
            client.print(interval1 - x);
            client.print("<br>");
            client.print("debug: ");
            client.print(mychar);
            client.print("<br>lights ");
	    client.print(stringone);
            client.print(mylight);
//            switch (mylight) {
//              case 0:
//              client.print(" Init");
//               break;
//              case 1:
//               client.print(" Halogen on");
//               break;
//              case 2:
//               client.print(" Room light on");
//               break;
//              case 3:
//               client.print(" Room light off");
//               break;
//              case 4:
//               client.print(" Dark");
//               break;
//              default:
//               client.print(" I dont know");
//               break;
//               
//            }
            client.print("<br>");
            client.print("c1: ");client.print(c1);client.print("c2: ");client.print(c2);client.print("<br>");
          for (int i = 0; i < 6; i++) {
           // client.print(millis()/1000);
            client.print(" analog input ");
            client.print(i);
            client.print(" is ");
            client.print(analogRead(i));
            client.print(" temprature is: ");
            client.print(get_temperature(i, myk[i]));
            sensor1Array[x] = get_temperature(i, myk[i]);
            client.println("<hr />");
          }
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
              client.print(convert2f(myeepromvalue, 1));
              }
              client.print("<br>");
              if (addr % 10 == 0){client.print("</tvd><td>");}   
            }
            client.print("</table>");
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
}


