/* My menu
 * Use petentiometer to map manu selections to be displayed on OLED
 * intialy from http://www.arduino.cc/en/Tutorial/Potentiometer
 * and the blink of led13 is a function of the pot
 * 5.11.2015 added some of myBridge Yun2
*/

#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>
#include <EEPROM.h>
#include <Streaming.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SeeedGrayOLED.h>
#include <avr/pgmspace.h>
#include <Wire.h>


#define ONE_WIRE_BUS 4
#define TEMPERATURE_PRECISION 12

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
// arrays to hold device addresses.  I need to change the order to keep my address and enumeration contant
DeviceAddress T1, T2, T3;
#define numberOfDevice  3;
DeviceAddress myT1Address =  { 0x28, 0x44, 0xDB, 0x5F, 0x06, 0x00, 0x00, 0x0A };






// Listen on default port 5555, the webserver on the Yún
// will forward there all the HTTP requests for us.
YunServer server;


int potPin = 0;    // select the input pin for the potentiometer
int ledPin = 13;   // select the pin for the LED
int val = 0;       // variable to store the value coming from the sensor
int oldMapVal = 0; // set the intital oldmap value


void setup() {

 pinMode(ledPin, OUTPUT);  // declare the ledPin as an OUTPUT
 Serial.begin(9600);
 Serial.print("Bridge Demo Yun2");
 


 //OLED Setup
  Wire.begin();
  SeeedGrayOled.init();             //initialize SEEED OLED display
  SeeedGrayOled.clearDisplay();     //Clear Display.
  SeeedGrayOled.setNormalDisplay(); //Set Normal Display Mode
  SeeedGrayOled.setVerticalMode();  // Set to vertical mode for displaying text

  SeeedGrayOled.setTextXY(0,0);  //set Cursor to ith line, 0th column
  SeeedGrayOled.setGrayLevel(10); //Set Grayscale level. Any number between 0 - 15.
  SeeedGrayOled.putString(" myBridgeYun"); //Print Hello World
  SeeedGrayOled.setTextXY(1,0);
  SeeedGrayOled.putString("  + Grove");
  SeeedGrayOled.setTextXY(2,0);
  SeeedGrayOled.putString("  + Oled");
  SeeedGrayOled.setTextXY(3,0);
  SeeedGrayOled.putString("  - Button");
  // print the reset of the menu
  for (int i=4; i<11; i++ ) {
    SeeedGrayOled.setTextXY(i,3);
    SeeedGrayOled.putNumber(i);
    SeeedGrayOled.putString(" menu"); 
  }

}

void loop() {

  val = analogRead(potPin);    // read the value from the sensor
  digitalWrite(ledPin, HIGH);  // turn the ledPin on
  //delay(val);                  // stop the program for some time
  digitalWrite(ledPin, LOW);   // turn the ledPin off
  //delay(val);                  // stop the program for some time
  int mapVal = map(val,990,0,0,12);
  if (mapVal != oldMapVal ) {
    //clear oldMapVal cursor
       SeeedGrayOled.setTextXY(oldMapVal,0); 
       SeeedGrayOled.putString(" ");
       oldMapVal = mapVal;
     
  }

    //Serial.print(mapVal);
    //Serial.print(" ");
    //Serial.println(val);
    
  // Draw the cursor 
    SeeedGrayOled.setTextXY(mapVal,0); 
    SeeedGrayOled.putString(">");
    
    // print the mapValue
     SeeedGrayOled.setTextXY(11,0); 
     SeeedGrayOled.putNumber(mapVal);
     
     // print the oldMapVal
     SeeedGrayOled.setTextXY(11,6); 
     SeeedGrayOled.putNumber(oldMapVal);
    
  

}
