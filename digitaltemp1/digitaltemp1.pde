

/*
counts the number of found devices
displays dallas 1wire temps for two sensors and also displays analog pin 1
used for photocell
7/14/2010 Kyle Bowerman
6/16/2011 Kyle
6/17 broken don't know why wont compile
*/

#include <OneWire.h>
#include <DallasTemperature.h>



// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 16

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

void setup(void)
{
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");

  // Start up the library
  sensors.begin();
}9600


void loop(void)
{ 
  //count the 1wire devices
  int devices =  sensors.getDeviceCount();
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  Serial.print(" Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  
  Serial.print("Devices found: ");
  Serial.println(devices);
  Serial.print("Temperature for Device 1 is: ");
  Serial.println(sensors.getTempFByIndex(0)); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  
  Serial.print("Temperature for Device 2 is: ");
  Serial.println(sensors.getTempFByIndex(1)); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  
 
  
  
  Serial.print("Anolog pin 1 ");
  Serial.println(analogRead(1));
  
  delay(2000);
}


