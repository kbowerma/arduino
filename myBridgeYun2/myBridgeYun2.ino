/*
  Arduino Yún Bridge example

 This example for the Arduino Yún shows how to use the
 Bridge library to access the digital and analog pins
 on the board through REST calls. It demonstrates how
 you can create your own API when using REST style
 calls through the browser.
 1/15/2015 Added one wite temp sensor
 5/8/2015 trying to add oled and run grove on top of yun2 had to move OneWire to bus to D4

 Possible commands created in this shetch:

 * "/arduino/digital/13"     -> digitalRead(13)
 * "/arduino/digital/13/1"   -> digitalWrite(13, HIGH)
 * "/arduino/analog/2/123"   -> analogWrite(2, 123)
 * "/arduino/analog/2"       -> analogRead(2)
 * "/arduino/mode/13/input"  -> pinMode(13, INPUT)
 * "/arduino/mode/13/output" -> pinMode(13, OUTPUT)

 This example code is part of the public domain

 http://arduino.cc/en/Tutorial/Bridge

Modified By Kyle Bowerman 1/6/2015

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

//----------- Setup ------------------

void setup() {

 //OLED Setup
  Wire.begin();
  SeeedGrayOled.init();             //initialize SEEED OLED display
  SeeedGrayOled.clearDisplay();     //Clear Display.
  SeeedGrayOled.setNormalDisplay(); //Set Normal Display Mode
  SeeedGrayOled.setVerticalMode();  // Set to vertical mode for displaying text

  SeeedGrayOled.setTextXY(0,0);  //set Cursor to ith line, 0th column
  SeeedGrayOled.setGrayLevel(10); //Set Grayscale level. Any number between 0 - 15.
  SeeedGrayOled.putString("myBridgeYun"); //Print Hello World
  SeeedGrayOled.setTextXY(1,0);
  SeeedGrayOled.putString("+ Grove");
  SeeedGrayOled.setTextXY(2,0);
  SeeedGrayOled.putString("+ Oled");
  SeeedGrayOled.setTextXY(3,0);
  SeeedGrayOled.putString("- Button");

 // Bridge startup
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);  // make this the alrm pin
    Bridge.begin();
    //digitalWrite(13, HIGH);
    // start serial port
    Serial.begin(9600);
    Serial.print("Bridge Demo Yun2");

    // Listen for incoming connection only from localhost
    // (no one from the external network could connect)
    server.listenOnLocalhost();
    server.begin();

    // Start up the library
    sensors.begin();

    // locate devices on the bus
    Serial << "Locating devices..." << "Found ";
    Serial.print(sensors.getDeviceCount(), DEC);
    Serial << " devices." << endl;



    // Check the devices are found and report their address
    if (!sensors.getAddress(T1, 0)) {
      Serial << "Unable to find address for Device T1" << endl;
      digitalWrite(13, HIGH);
    } else {
      Serial << "the address of device T1 is:" << endl;
      printAddress(T1);
      sensors.setResolution(T1, TEMPERATURE_PRECISION);
      Serial << endl << " the resolution is: ";
      Serial.println(sensors.getResolution(T1), DEC);
      Serial << "the temp is " << getTempF(T1);
    }
    if (!sensors.getAddress(T2, 1)) {
      Serial << "Unable to find address for Device T2" << endl;
      digitalWrite(13, HIGH);
      } else {
        Serial << endl << "the address of device T2 is:" << endl;
        printAddress(T2);
        sensors.setResolution(T2, TEMPERATURE_PRECISION);
        Serial << endl << "the resolution is " << endl;
        Serial.println(sensors.getResolution(T2), DEC);
        Serial << "the temp is " << getTempF(T2);
      }
      if (!sensors.getAddress(T3, 2)) {
        Serial << "Unable to find address for Device T3" << endl;
        digitalWrite(13, HIGH);
        } else {
          Serial << endl << "the address of device T3 is:" << endl;
          printAddress(T3);
          sensors.setResolution(T3, TEMPERATURE_PRECISION);
          Serial << endl << "the resolution is " << endl;
          Serial.println(sensors.getResolution(T3), DEC);
          Serial << "the temp is " << getTempF(T3);
        }

      // set the resolution to 9 bit
      sensors.setResolution(T1, TEMPERATURE_PRECISION);
      sensors.setResolution(T2, TEMPERATURE_PRECISION);
      sensors.setResolution(T3, TEMPERATURE_PRECISION);


 }



  // --------- functions ---------

  // function to print a device address
  void printAddress(DeviceAddress deviceAddress) {
    for (uint8_t i = 0; i < 8; i++)
    {
      // zero pad the address if necessary
      if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
    }
  }

  void clientPrintAddress(DeviceAddress deviceAddress, YunClient client) {
    for (uint8_t i = 0; i < 8; i++)
    {
      // zero pad the address if necessary
      if (deviceAddress[i] < 16) client.print("0");
      client.print(deviceAddress[i], HEX);
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

  //function to get temp, note it reuruns a value so you need to print it
  float getTempF(DeviceAddress deviceAddress) {
    float tempC = sensors.getTempC(deviceAddress);
    Serial << "from the function "  << DallasTemperature::toFahrenheit(tempC);
    Serial << " c temp is " << tempC << endl;
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



//----------- loop ----------

void loop() {
  // call sensors.requestTemperatures() to issue a global temperature
  sensors.requestTemperatures();



  //Set up serial responder
  if (Serial.available() > 0) {
    int inByte = Serial.read();
    switch(inByte) {
      case 'i':
       Serial << endl << "This is myBridgeYun2 1/6/2015" << endl << " 1/15 Adding temp sensor " << "1/29 add thrid temp sensor" << endl;
       // locate devices on the bus
       Serial << "Locating devices..." << "Found ";
       Serial.print(sensors.getDeviceCount(), DEC);
       Serial << " devices." << endl;
      break;
      case 'e':
       printEEPROM();
      break;
      case '1':
        Serial << endl << getTempF(T1) << endl;
      break;
      case '2':
        Serial << endl << getTempF(T2) << endl;
      break;
      case 'p':
      //T1
       Serial << endl << "the temp T1 is " << endl;
       printTemperature(T1);
       Serial << endl << "the address for T1 is ";
       printAddress(T1);
       //T2
       Serial << endl << "the temp T2 is " << endl;
       printTemperature(T2);
       Serial << endl << "the address for T2 is ";
       printAddress(T2);
       //T2
       Serial << endl << "the temp T3 is " << endl;
       printTemperature(T3);
       Serial << endl << "the address for T3 is ";
       printAddress(T3);
      break;
      case 'u':
        Serial << endl << millis() / 1000;
      break;
      case 't':
        Serial << endl << "the temp myT1Address, called by Address (printTemperature(myT1Address); )  is " << endl;
        printTemperature(myT1Address);
      break;


    }
  }

  // Get clients coming from server
  YunClient client = server.accept();

  // There is a new client?
  if (client) {
    // Process request
    process(client);

    // Close connection and free resources.
    client.stop();
  }

  delay(50); // Poll every 50ms
}

void process(YunClient client) {
  // read the command
  String command = client.readStringUntil('/');

  // is "digital" command?
  if (command == "digital") {
    digitalCommand(client);
  }

  // is "analog" command?
  if (command == "analog") {
    analogCommand(client);
  }

  // is "mode" command?
  if (command == "mode") {
    modeCommand(client);
  }
  if (command == "check") {
    client << "this is test" << endl;
    client << "Light A0: " << analogRead(0) << endl;
    client << "Moisture A1: " << analogRead(1) << endl;
  }
  if (command == "temp") {
    //client << "the temp is "  << getTempF(T1) << endl;
    tempCommand(client);
  }
}

void tempCommand(YunClient client){
  //type DeviceAddress value: deviceAddress
  //DeviceAddress clientDeviceAddress;

  String clientRequestDevice = client.readStringUntil('\r');
  if (clientRequestDevice == "1") {
    client << "{\"temp\": " << getTempF(T1) << "," << endl;
    client << " \"device\": \"";
    clientPrintAddress(T1, client);
    client << "\"}";
    }
    if (clientRequestDevice == "2") {
      client << "{\"temp\": " << getTempF(T2) << "," << endl;
      client << " \"device\": \"";
      clientPrintAddress(T2, client);
      client << "\"}";
    }
  return;

}

void digitalCommand(YunClient client) {
  int pin, value;

  // Read pin number
  pin = client.parseInt();

  // If the next character is a '/' it means we have an URL
  // with a value like: "/digital/13/1"
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
  }
  else {
    value = digitalRead(pin);
  }

  // Send feedback to client
  client.print(F("Pin D"));
  client.print(pin);
  client.print(F(" set to "));
  client.println(value);

  // Update datastore key with the current pin value
  String key = "D";
  key += pin;
  Bridge.put(key, String(value));
}

void analogCommand(YunClient client) {
  int pin, value;

  // Read pin number
  pin = client.parseInt();

  // If the next character is a '/' it means we have an URL
  // with a value like: "/analog/5/120"
  if (client.read() == '/') {
    // Read value and execute command
    value = client.parseInt();
    analogWrite(pin, value);

    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" set to analog "));
    client.println(value);

    // Update datastore key with the current pin value
    String key = "D";
    key += pin;
    Bridge.put(key, String(value));
  }
  else {
    // Read analog pin
    value = analogRead(pin);

    // Send feedback to client
    client.print(F("Pin A"));
    client.print(pin);
    client.print(F(" reads analog "));
    client.println(value);

    // Update datastore key with the current pin value
    String key = "A";
    key += pin;
    Bridge.put(key, String(value));
  }
}

void modeCommand(YunClient client) {
  int pin;

  // Read pin number
  pin = client.parseInt();

  // If the next character is not a '/' we have a malformed URL
  if (client.read() != '/') {
    client.println(F("error"));
    return;
  }

  String mode = client.readStringUntil('\r');

  if (mode == "input") {
    pinMode(pin, INPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as INPUT!"));
    return;
  }

  if (mode == "output") {
    pinMode(pin, OUTPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as OUTPUT!"));
    return;
  }

  client.print(F("error: invalid mode "));
  client.print(mode);
}
