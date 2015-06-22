/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the Uno and
  Leonardo, it is attached to digital pin 13. If you're unsure what
  pin the on-board LED is connected to on your Arduino model, check
  the documentation at http://arduino.cc

  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
 */


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  delay(1000);
  Serial.print("micro/myBlink \n 6/9/2015");
}

// the loop function runs over and over again forever
void loop() {

  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(500);     // wait for a second
   digitalWrite(13, HIGH);  
  delay(100);  
     digitalWrite(13, LOW);  
  delay(100);  
  Serial.print(millis());
  
  if (Serial.available() > 0) {
    int inByte = Serial.read();
    
    switch (inByte) {
    case 'i':
     Serial.print("micro/myBlink \n 6/9/2015");
     break;
    case 'm':
     Serial.print("\n\ti\tinfo\n\tm\tprint menu\n\tu\tuptime\n");
    break;
   case 'u':
    Serial.print(millis()/1000);
   break; 
    }
      
  }
}
