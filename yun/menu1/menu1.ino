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
DeviceAddress insideThermometer;






// Listen on default port 5555, the webserver on the Yún
// will forward there all the HTTP requests for us.
YunServer server;


int potPin = 0;    // select the input pin for the potentiometer
int ledPin = 13;   // select the pin for the LED
int pumpPin = 8;
int val = 0;       // variable to store the value coming from the sensor
int oldMapVal = 0; // set the intital oldmap value
const int buttonPin = 5;
float myTemp = 0;
char*  mmode = "none";
char* mainArr[] = {"Main", "Off", "clear", "temp", "uptime","pump","six","seven","eight", "nine","ten"};



void setup() {

 pinMode(ledPin, OUTPUT);  // declare the ledPin as an OUTPUT
 pinMode(pumpPin, OUTPUT);
 Serial.begin(9600);
 Serial.print("Bridge Demo Yun2");

 //OLED Setup
  Wire.begin();
  oledClear();
  splashScreen();
  delay(4000);
  oledClear();
  mainMenu();

  //1 wire temprature
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

  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");

  sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);

}

void loop() {
  int mapVal = map(val,990,0,0,12);
  //sensors.requestTemperatures();
  //myTemp = DallasTemperature::toFahrenheit(sensors.getTempC(insideThermometer));

  val = analogRead(potPin);    // read the value from the sensor
  digitalWrite(ledPin, HIGH);  // turn the ledPin on
  //delay(val);
  if (digitalRead(buttonPin) == HIGH ) {
    // turns on the oled as long as the pot is not on 4
    oledOn();
    //Turn off the LED with the button
    digitalWrite(ledPin, LOW);   // turn the ledPin off
    if (mapVal == 0 ) mainMenu();
    if (mapVal == 1 ) oledOff();
    if (mapVal == 2 ) oledClear();
    if (mapVal == 3 ) oledTemp1();
    if (mapVal == 4 ) oledUptime();
    if (mapVal == 5 ) runPump();
  } else {
  // Turn off Pump
  digitalWrite(pumpPin, LOW);
   }

  if (mapVal != oldMapVal ) {
    //clear oldMapVal cursor
       SeeedGrayOled.setTextXY(oldMapVal,0);
       //SeeedGrayOled.sendCommand(SeeedGrayOLED_Normal_Display_Cmd);
       SeeedGrayOled.putString(" ");
       oldMapVal = mapVal;
  }


  // Draw the cursor
    SeeedGrayOled.setTextXY(mapVal,0);
    //SeeedGrayOled.sendCommand(SeeedGrayOLED_Inverse_Display_Cmd);
    SeeedGrayOled.putString(">");

    // print the mapValue
     SeeedGrayOled.setTextXY(11,0);
     SeeedGrayOled.putNumber(mapVal);
     // clear the zero after printing 10
     if (mapVal < 10 ) {
       SeeedGrayOled.setTextXY(11,1);
       SeeedGrayOled.putString(" ");
     }

     // print the oldMapVal
     SeeedGrayOled.setTextXY(11,3);
     mmode = mainArr[mapVal];
     SeeedGrayOled.putString(mmode);


}




// functions
void runPump() {
  digitalWrite(pumpPin, HIGH);
}


void oledOff() {
  SeeedGrayOled.sendCommand(0xAE);
}
void oledOn() {
  SeeedGrayOled.sendCommand(0xAF);
}

void oledClear() {
 SeeedGrayOled.init();
 SeeedGrayOled.clearDisplay();
}


void oledTemp1(){
  sensors.requestTemperatures();
  myTemp = DallasTemperature::toFahrenheit(sensors.getTempC(insideThermometer));
  oledClear();
  SeeedGrayOled.setTextXY(0,1);
  SeeedGrayOled.putString("0 Main");
  SeeedGrayOled.setTextXY(1,1);
  SeeedGrayOled.putString("3 Temp1");
  SeeedGrayOled.setTextXY(2,0);
  SeeedGrayOled.putString("============");
  SeeedGrayOled.setTextXY(3,2);
  SeeedGrayOled.putNumber(myTemp);

}

void oledUptime(){
  oledClear();
  SeeedGrayOled.setTextXY(0,1);
  SeeedGrayOled.putString("0 Main");
  SeeedGrayOled.setTextXY(1,1);
  SeeedGrayOled.putString("4 uptime");
  SeeedGrayOled.setTextXY(2,0);
  SeeedGrayOled.putString("============");
  SeeedGrayOled.setTextXY(3,1);
  int uptime = millis();
  if (uptime > 1000 && uptime < 60000) {
    SeeedGrayOled.clearDisplay();
    SeeedGrayOled.putNumber(millis()/1000);
    SeeedGrayOled.putString("  secs");
  }
  if (uptime > 60000 ) {
    SeeedGrayOled.clearDisplay();
     SeeedGrayOled.putNumber(millis()/60000);
     SeeedGrayOled.putString("  mins");
   }
}



void mainMenu() {
  SeeedGrayOled.init();             //initialize SEEED OLED display
  SeeedGrayOled.clearDisplay();     //Clear Display.
  SeeedGrayOled.setNormalDisplay(); //Set Normal Display Mode
  SeeedGrayOled.setVerticalMode();  // Set to vertical mode for displaying text

  SeeedGrayOled.setTextXY(0,0);  //set Cursor to ith line, 0th column
  SeeedGrayOled.setGrayLevel(10); //Set Grayscale level. Any number between 0 - 15.

  for (int i=0; i<11; i++){
    SeeedGrayOled.setTextXY(i,1);
    SeeedGrayOled.putNumber(i);
    SeeedGrayOled.putString(" ");
    SeeedGrayOled.putString(mainArr[i]);
  }


}

void splashScreen() {
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
  SeeedGrayOled.putString("  + Main");
  SeeedGrayOled.setTextXY(4,0);
  SeeedGrayOled.putString("5/19/2015");
}
