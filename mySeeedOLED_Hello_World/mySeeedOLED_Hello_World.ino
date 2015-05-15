#include <Wire.h>
#include <SeeedGrayOLED.h>


//For display on off button
const int buttonPin = 4;    // the number of the pushbutton pin
// Variables will change:
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button


void setup()
{
  pinMode(buttonPin, INPUT);
  Serial.begin(9600);        // start serial for output
  Serial.print("mySeeedOLED_Hello_World");
  
  Wire.begin();
  SeeedGrayOled.init();             //initialize SEEED OLED display
  SeeedGrayOled.clearDisplay();     //Clear Display.
  SeeedGrayOled.setNormalDisplay(); //Set Normal Display Mode
  SeeedGrayOled.setVerticalMode();  // Set to vertical mode for displaying text
  
  
  for(char i=0; i < 13 ; i++)
  {
  SeeedGrayOled.setTextXY(i,0);  //set Cursor to ith line, 0th column
  SeeedGrayOled.setGrayLevel(i); //Set Grayscale level. Any number between 0 - 15.
  SeeedGrayOled.putNumber(i); //print the line
  SeeedGrayOled.putString("Hello WRLd"); //Print Hello World
  }
 
  
}

void loop()
{

  
  unsigned long currentMillis = millis();  //Blink without delay
  
 
 // myClearDisplay();     //Clear Display.
 
  buttonState = digitalRead(buttonPin);


  //delay(1000);
  SeeedGrayOled.setTextXY(0,0);
   SeeedGrayOled.putString("Nolen ");
   SeeedGrayOled.putNumber(buttonPushCounter);
   

   
  if (buttonState != lastButtonState) {
  // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      buttonPushCounter++; 
      buttonChange();
      
    }
    lastButtonState = buttonState;
  }    
   
} // loop
 
 // -------- functions
 
 void buttonChange() {
   
   if ( buttonPushCounter % 3 == 1 ) {
    //SeeedGrayOled.init();  
          SeeedGrayOled.setTextXY(3,0);
          SeeedGrayOled.setGrayLevel(15);
          SeeedGrayOled.putString("Button ON");
          Serial.print("button on");
           
      } 
   if ( buttonPushCounter % 3 == 2 ) {
        SeeedGrayOled.setTextXY(3,0);
        SeeedGrayOled.setGrayLevel(1);
        SeeedGrayOled.putString("Button OFF"); 
        Serial.print("button off");
      }  
      
      if ( buttonPushCounter % 3 == 0 ) {
        SeeedGrayOled.setTextXY(0,3);
         myClearDisplay();     //Clear Display.
         SeeedGrayOled.setTextXY(3,0);
         SeeedGrayOled.putString("CLR"); 
          SeeedGrayOled.init();
        Serial.print("clear called");
      }     
   
 }
 
 void emptyDisplay() {
  for (char x=0; x<12; x++) {
    SeeedGrayOled.setTextXY(x,0);
    SeeedGrayOled.clearDisplay(); 
    
  }
 }
 
 void  turnOffOLED() {
   SeeedGrayOled.sendCommand(0xAE); // Set display off
 }
  void  turnOnOLED() {
  SeeedGrayOled.init();             //initialize SEEED OLED display
 }
 
 void myClearDisplay()
{
    unsigned char i,j;
    for(j=0;j<96;j++)
    {
        for(i=0;i<96;i++)  //clear all columns
        {
            mySendData(0x00);
        }
    }

}

void mySendData(unsigned char Data)
{
    Wire.beginTransmission(SeeedGrayOLED_Address); // begin I2C transmission
    Wire.write(SeeedGrayOLED_Data_Mode);            // data mode
    Wire.write(Data);
    Wire.endTransmission();                    // stop I2C transmission
}
  
  



