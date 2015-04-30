// Wire Master Writer
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Writes data to an I2C/TWI slave device
// Refer to the "Wire Slave Receiver" example for use with this

// Created 29 March 2006

// This example code is in the public domain.
// kyle this is the master on the duemilonve /dev/usbserial-A7005NYT
// added blink 4/18/2015


#include <Wire.h>

void setup()
{
  Wire.begin(5); // join i2c bus (address optional for master)
    // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  Serial.println("i2c master example");
  Serial.println("Kyle Bowerman 4/20/2015");
}

byte x = 0;

void loop()
{
   delay(500);
  Wire.beginTransmission(4); // transmit to device #4
  Wire.write("x is ");        // sends five bytes
  Wire.write(x);              // sends one byte  
  Wire.endTransmission();    // stop transmitting
   digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)

  x++;
  delay(500);
   digitalWrite(13, LOW);   // turn the LED on (HIGH is the voltage level)
}
