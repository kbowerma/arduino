/*
 * Name: myMultiLeowButton
 * was  : TH02_dev demo
 * Usage       : DIGITAL I2C HUMIDITY AND TEMPERATURE SENSOR
 * Author      : Oliver Wang from Seeed Studio
 * Version     : V0.1
*/

#include <TH02_dev.h>
#include "Arduino.h"
#include "Wire.h"
#include <Streaming.h>

#include "rgb_lcd.h"

rgb_lcd lcd;
const int buttonPin = 4;    // the number of the pushbutton pin

int m1Pin = A0;
float m1Value = 0;
float m1ValuePercent = 0;
int displayMode = 1;
// displayMode 1 = Auto
// displayMode 0 = Off

int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers


void setup()


{

  pinMode(A0, INPUT);
  pinMode(buttonPin, INPUT);

  Serial.begin(9600);        // start serial for output

  Serial.println("****TH02_dev demo by seeed studio****\n");
  /* Power up,delay 150ms,until voltage is stable */
  delay(150);
  /* Reset HP20x_dev */
  TH02.begin();
  lcd.begin(16, 2);
    // Print a message to the LCD.
  lcd.print("myTHO2 demo");
  delay(100);

  /* Determine TH02_dev is available or not */
  Serial.println("TH02_dev is available.\n");



}


void loop()
{
  unsigned long currentMillis = millis();  //Blink without delay

  //Begin Button Logic - see Debounce Example

  int reading = digitalRead(buttonPin);
  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    Serial << "Button Pressed the displayMode is " << displayMode << endl;
    Serial << "reading is " << reading << endl;
    Serial << "lastButtonState " << lastButtonState << endl;
    Serial << "buttonState " << buttonState << endl;
    Serial << "lastDebounceTime " << lastDebounceTime << endl;
    //Serial.println(displayMode);
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:


    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
      Serial << "Button Pressed at " << millis()/1000 << endl;

      // There been a change do some stuff
      if (displayMode == 1 ) {
        displayMode = 0;
      }
      else if (displayMode == 0 ) {
        displayMode = 1;
      }
    }
  }

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
    lastButtonState = reading;


  //End Button Logic

  if (displayMode == 1 ) {
    //delay(5000);
    if (currentMillis % 5000 == 0 ) doTemp();
    //delay(5000);
    if (currentMillis % 10000 == 0 ) doHumidity();
    //delay(5000);
    if (currentMillis % 20000 == 0 ) doMoisture();
    //delay(5000);
    //lcd.clear(); // clear the moisture for the lcd
    //lcd.print(millis());  //print the millis
    if  (currentMillis % 7000 == 0 ) {
      Serial << "DEBUG" << "Button Pressed the displayMode is " << displayMode << endl;
      Serial << "DEBUG" << "reading is " << reading << endl;
      Serial << "DEBUG" << "lastButtonState " << lastButtonState << endl;
      Serial << "DEBUG" << "buttonState " << buttonState << endl;
      Serial << "DEBUG" << "lastDebounceTime " << lastDebounceTime << endl;
    }
  }

  if (displayMode == 0 )  closeDisplay();
  if (displayMode == 1 )  openDisplay();



}

//---------------- End main

void doTemp () {
     // add a delay at 5 deconds
   //delay(5000);
   float temper = TH02.ReadTemperature();
   Serial.println("Temperature: ");
   //Serial.print(temper);
   float temperF = (temper * 9/5) +32;
   Serial.print(temperF);
   Serial.println("F\r\n");
   lcd.clear();
   lcd.print(temperF);
   lcd.print(" degrees F");
   lcd.setRGB(255,100,100);
   if ( temperF > 72 ) {
     lcd.setRGB(255,0,0);
   }
}

void doHumidity() {
     float humidity = TH02.ReadHumidity();
   Serial.println("Humidity: ");
   Serial.print(humidity);
   Serial.println("%\r\n");
   lcd.clear();
   lcd.print(humidity);
   lcd.print(" % humidity");
   lcd.setRGB(100,255,100);
}

void doMoisture() {
     m1Value = analogRead(m1Pin);
   lcd.setRGB(100,100,255);
   Serial.println("Moisture m1: ");
   Serial.print(m1Value);
   Serial.print(" / ");
   m1ValuePercent = m1Value * 100 / 1024;
   Serial.print((int) m1ValuePercent );
   Serial.println("%\r\n");

   lcd.clear();
   lcd.print("moisture ");
   lcd.print((int) m1ValuePercent);
   lcd.print(" %");
}

void closeDisplay() {
   lcd.clear();
   lcd.setRGB(255,255,255);
   lcd.print("turning off display in 5 seconds");
   delay(5000);
   lcd.noDisplay();
   lcd.setRGB(0,0,0);

}

void openDisplay() {
  lcd.display();
}
