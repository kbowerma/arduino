
/*
counts the number of found devices
displays dallas 1wire temps for two sensors and also displays analog pin 1
used for photocell
7/14/2010 Kyle Bowerman

*/




#include <OneWire.h>
#include <DallasTemperature.h>



// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

void setup(void)
{
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");
  pinMode(3, OUTPUT);

  // Start up the library
  sensors.begin();
}


void loop(void)
{ 
  //count the 1wire devices
  int devices =  sensors.getDeviceCount();
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  
  
  
  
  //----- start relay on off.
  if (Serial.available() > 0) {
    
    int inByte = Serial.read();
    
    switch (inByte) {
     //On      
      case '1':
       digitalWrite(3, HIGH);
       Serial.println("Turning Relay on");
       break;
     //off 
      case '0':
       digitalWrite(3,LOW);
       Serial.println("turing relay off");
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
        
       
        
        
        Serial.print("Anolog pin 0 ");
        Serial.println(analogRead(0));
        Serial.print("Anolog pin 1* ");
        Serial.println(analogRead(1));
        Serial.print("Anolog pin 2 ");
        Serial.println(analogRead(2));
      break;
      
      default:
       digitalWrite(3,LOW);  
    }  
  }
    
  
  
  delay(2000);
}


