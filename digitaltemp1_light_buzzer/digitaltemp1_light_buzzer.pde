
/*
counts the number of found devices
displays dallas 1wire temps for two sensors and also displays analog pin 1
used for photocell
******** Brick
A1 photcell 
A2 temprature LM358P(op amp)
D10 buzzer
7/14/2010 Kyle Bowerman
6/16/2011 Kyle Bowerman
 gets the digitial temps and x prints the 1wire address.
 
8/1 this works for reading the temp but when I move the 1wire coe to MegaEth_dtemp_v2 I dont read the temp 

*/



#include <OneWire.h>
#include <DallasTemperature.h>


// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 53

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

void setup(void)
{
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");
  Serial.println("File: digitaltemp1_light_buzzer");
  pinMode(10, OUTPUT);
  

  // Start up the library
  sensors.begin();
}


void loop(void)
{ 
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
        
       for (int x = 0; x < 5; x++ ){
         Serial.print("Analog pin ");
         Serial.print(x);
         Serial.print(" ");
         Serial.println(analogRead(x));
         
       }
       
        
//        
//        Serial.print("Anolog pin 0 ");
//        Serial.println(analogRead(0));
//        Serial.print("Anolog pin 1* ");
//        Serial.println(analogRead(1));
//        Serial.print("Anolog pin 2 ");
//        Serial.println(analogRead(2));
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
}


