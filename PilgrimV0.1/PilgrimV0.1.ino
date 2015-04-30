/*Kyle Bowerman 12/23/2014, 12/27/2014
From: File PilgrimV0.1.ino
  + web server
*/
#include <OneWire.h>
#include <Streaming.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EEPROM.h>
#include "Statistic.h"

Statistic myStats;



// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 3
#define TEMPERATURE_PRECISION 9
#define SKETCHNAME "PilgrimV0.2.ino"


// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress insideThermometer, outsideThermometer, externalThermometer;

//ethernet
byte mac[]  = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 11, 82);
EthernetServer server(80);

// moisture sensor
int moistureSensor = 4;
int moisture_val;
float moisture_volts = moisture_val * 5.0 /1024 ;
int debugTemp = 1;
int debugWeb =1;
int lc =0;
int slot =0;
float mArray[10];


void setup(void) {
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");
  Serial.print("File: ");
  Serial.print(SKETCHNAME);

  myStats.clear(); //explicitly start clean


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());


  // Start up the library
  sensors.begin();

  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  //
  // method 1: by index
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
  if (!sensors.getAddress(outsideThermometer, 1)) Serial.println("Unable to find address for Device 1");
  if (!sensors.getAddress(externalThermometer, 2)) Serial.println("Unable to find address for Device 2");


  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();

  Serial.print("Device 1 Address: ");
  printAddress(outsideThermometer);
  Serial.println();

  // set the resolution to 9 bit
  sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
  sensors.setResolution(outsideThermometer, TEMPERATURE_PRECISION);
  sensors.setResolution(externalThermometer, TEMPERATURE_PRECISION);


  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC);
  Serial.println();

  Serial.print("Device 1 Resolution: ");
  Serial.print(sensors.getResolution(outsideThermometer), DEC);
  Serial.println();

  Serial.print("Device 2 Resolution: ");
  Serial.print(sensors.getResolution(externalThermometer), DEC);
  Serial.println();
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress) {
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
}

// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress) {
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();
}

// main function to print information about a device
void printData(DeviceAddress deviceAddress) {
  Serial.print("Device Address: ");
  printAddress(deviceAddress);
  Serial.print(" ");
  printTemperature(deviceAddress);
  Serial.println();
}

float getTempF(DeviceAddress deviceAddress) {
  float tempC = sensors.getTempC(deviceAddress);
    return DallasTemperature::toFahrenheit(tempC);
}

void printEEPROM(){
  Serial.print(" printing the EEPROM");
  for(int x = 0; x < 512; x++){
    Serial.print(x);
    Serial.print('\t');
    Serial.print(EEPROM.read(x));
    Serial.println();
  }
}

void clearEEPROM(){
  for(int k=1; k <512; k++){  // clear the EEPROM
    EEPROM.write(k,0);
  }
}

void displayMenu() {
  Serial << endl << "Serial Input menu:" << endl;
  Serial << " e = print the eeprom " << endl;
  Serial << " c = clear the eeprom: " << endl;
  Serial << " t = toggle the temprature debug " << endl;
  Serial << " w = toggle the Webserver debug " << endl;
  Serial << " m = print this menu " << endl;

}

//----------- Begin Loop --------------

void loop(void) {

  //increment loopcounter lc
  lc++;
  slot++;
  if (slot == 10) slot =0;

 // List for serial commands
 if (Serial.available() > 0) {
   int inByte = Serial.read();
   switch (inByte) {
     case 'e': //print eeprom
      printEEPROM();
     break;
     case 'c': //clear eeprom
      clearEEPROM();
      Serial.println("cleared the EEPROM");
     break;
      case 'm':
        displayMenu();
      break;
      case 't':  //toggle temp debug
        if (debugTemp==1) {
          debugTemp = 0;
          Serial.println("Temp debugging off");
        } else {
          debugTemp = 1;
          Serial.println("Temp debugging ON");
        }
      break;
      case 'w':
        if (debugWeb==1) {
          debugWeb=0;
          Serial << " Web debugging turrned off " << endl;
        } else {
          debugWeb=1;
          Serial << " Web debugging turned on " << endl;
        }
      break;

    default:
      Serial.println("Type m for input command help");
   }
 }




  //standard delay for me
  delay(5000);
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  if (debugTemp == 1 ) {
    Serial.print("Requesting temperatures...");
    sensors.requestTemperatures();
    Serial.println("DONE");

    // print the device information
    printData(insideThermometer);
    printData(outsideThermometer);
    printData(externalThermometer);
  }

  //print the moisture sensure value
  moisture_val = analogRead(moistureSensor);
  Serial << lc  << "\tmoisture: " << moisture_val << "\t light: " << analogRead(A2) << " slot " << slot << endl;
  mArray[slot] = moisture_val;
  myStats.clear();
  for (int x = 0; x<10; x++){
    Serial << mArray[x] << "\t";
    myStats.add(mArray[x]*10);
  }
  Serial << endl;
  //myStats.add(mArray);
  Serial << "average: " << myStats.average()/10 << endl;
  Serial << "count: " << myStats.count() << endl;
  Serial << "std dev: " << myStats.pop_stdev() << endl;




  //Web server
    // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    if(debugWeb==1) Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if(debugWeb==1) Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.print("File: ");
          client.print(SKETCHNAME);
          client.println("<hr />");
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
          }

          client << "<hr/>" << endl;
          client << " insideThermometer: " << getTempF(insideThermometer) << "<br/>" << endl;
          client << " outsideThermometer: " << getTempF(outsideThermometer) << "<br/>" << endl;
          client << " externalThermometer: " << getTempF(externalThermometer) << "<br/>" << endl;

          client.println("<hr />");
          client.print("Moisture (A4) at: ");
          client.print( moisture_val);
          client.print(" volts ");
          client.print(float(moisture_val)*5/1023);

          client.print("<br/>Light (A2)      at: ");
          client.print( analogRead(A2));
          client.print(" volts ");
          client.print(float(analogRead(A2))*5/1023);



          client.println("</html>");
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
    delay(1);
    // close the connection:
    client.stop();
    if(debugWeb==1) Serial.println("client disconnected");
  }








}
