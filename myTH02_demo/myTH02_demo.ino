/*
 * Demo name   : TH02_dev demo 
 * Usage       : DIGITAL I2C HUMIDITY AND TEMPERATURE SENSOR 
 * Author      : Oliver Wang from Seeed Studio
 * Version     : V0.1
*/

#include <TH02_dev.h>
#include "Arduino.h"
#include "Wire.h" 

#include "rgb_lcd.h"

rgb_lcd lcd;
int m1Pin = A0;
float m1Value = 0;
float m1ValuePercent = 0;
int displayMode = 1;

 
void setup()
{  
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
  
  pinMode(A0, INPUT);

   
}
 

void loop()
{
  
  delay(5000);
  doTemp();   
  delay(5000);
  doHumidity();
  delay(5000);
  doMoisture();
   
   
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

void closeDisplay(){
   lcd.clear();
   lcd.setRGB(255,255,255);
   lcd.print("turning off display in 5 seconds");
   lcd.noDisplay();
  
}

void openDisplay(){
  lcd.display()
}
 


 
