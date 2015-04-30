/*
 * Name: myMultiLeowButton
 * was  : TH02_dev demo
 * Usage       : DIGITAL I2C HUMIDITY AND TEMPERATURE SENSOR
 *             : seedstudio RGB Serial LCD (https://github.com/Seeed-Studio/Grove_LCD_RGB_Backlight)
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
String displayMode = "auto";
boolean DEBUG = false;
// displayMode 1 = Auto
// displayMode 0 = Off

// Variables will change:
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button


void setup() {

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
  lcd.print("myMultiLeowButton");
  delay(100);

  /* Determine TH02_dev is available or not */
  Serial.println("TH02_dev is available.\n");
}


void loop()
{
  unsigned long currentMillis = millis();  //Blink without delay


  checkButton();

  if  (currentMillis % 7000 == 0 && DEBUG == true) {
    Serial << "DEBUG: " << "buttonPushCounter= " << buttonPushCounter << endl;
    Serial << "DEBUG: " << "displayMode= " << displayMode << endl;

  }


  if (displayMode == "auto" ) {
    if (currentMillis % 5000 == 0 ) doTemp();
    if (currentMillis % 10000 == 0 ) doHumidity();
    if (currentMillis % 20000 == 0 ) doMoisture();
  }



}

//---------------- End main

void checkButton() {
  // read the pushbutton input pin:
   buttonState = digitalRead(buttonPin);

   // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
  // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      // if the current state is HIGH then the button
      // wend from off to on:
      buttonPushCounter++;
      Serial.println("on");
      Serial.print("number of button pushes:  ");
      Serial.println(buttonPushCounter);

     // change to switch case
      if ( buttonPushCounter  == 0 ) displayMode = "auto";  //logic in main
      if ( buttonPushCounter == 1 ) {
        displayMode = "off";
        closeDisplay();
      }
      if ( buttonPushCounter == 2 ) {
        displayMode = "temp";
        doTemp();
        }
      if ( buttonPushCounter == 3 ) {
        displayMode = "humdity";
        doHumidity();
        }
      if ( buttonPushCounter == 4 ) {
        displayMode = "moisture";
        doMoisture();
      }

      if ( buttonPushCounter == 5 ) {
        buttonPushCounter = 0;
        displayMode = "auto";
        //  no function to call auto is part of the main loop


    }
    else {
      // if the current state is LOW then the button
      // wend from on to off:
      Serial.println("off");
    }
  }
  // save the current state as the last state,
  //for next time through the loop
  Serial << "DEBUG: " << "buttonPushCounter= " << buttonPushCounter << endl;
  Serial << "DEBUG: " << "displayMode= " << displayMode << endl;
  lastButtonState = buttonState;
 }
}



void doTemp() {
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
   delay(500);
   //lcd.noDisplay();
   lcd.setRGB(0,0,0);

}

void openDisplay() {
  lcd.display();
}
