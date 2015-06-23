/* From: Encoder Library - Basic Example
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 *
 *
 * 6.22.2015  ktb slowly adding microled
 * SOLVED --checking that newPosition % 4 = 0 fixed rollback issue
 */


 #include <Wire.h>  // Include Wire if you're using I2C
 #include <SPI.h>  // Include SPI if you're using SPI
 #include <SFE_MicroOLED.h>  // Include the SFE_MicroOLED library
 //#define ENCODER_DO_NOT_USE_INTERRUPTS
 #include <Encoder.h> // enocer libarary

 //////////////////////////
 // MicroOLED Definition //
 //////////////////////////
 #define PIN_RESET 9  // Connect RST to pin 9
 #define PIN_DC    8  // Connect DC to pin 8
 #define PIN_CS    10 // Connect CS to pin 10
 #define DC_JUMPER 1



 //////////////////////////////////
 // MicroOLED Object Declaration //
 //////////////////////////////////
 //MicroOLED oled(PIN_RESET, PIN_DC, PIN_CS); // SPI declaration
 MicroOLED oled(PIN_RESET, DC_JUMPER);    // I2C declaration

// Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
Encoder myEnc(7, 6);
//   avoid using pins with LEDs attached
int newtime = 0;
int oldtime = 0;

void setup() {
  oled.begin();    // Initialize the OLED
  oled.clear(ALL); // Clear the display's internal memory
  //oled.display();  // Display what's in the buffer (splashscreen)
//  delay(1000);     // Delay 1000 ms
  oled.clear(PAGE); // Clear the buffer.
  Serial.begin(9600);
  Serial.println("Basic Encoder Test:");
}

long oldPosition  = -999;

void loop() {
  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldtime = newtime;
    newtime = millis();
    oldPosition = newPosition;
    Serial.println(newPosition);
    // only call the oled on the terminal state, it appears to screw
    if (newPosition % 4 == 0 ) {
      // add a little hold time timer to stop bounce
      if (newtime - oldtime > 3) {
        showPos(newPosition);
      }
      else {
        Serial.print("timediff: ");
        Serial.println(newtime - oldtime);
      }
    }

  }
  //delay(10);
}

void showPos(long pos) {
  oled.clear(PAGE);            // Clear the display
  oled.setCursor(0, 0);        // Set cursor to top-left
  oled.setFontType(0);
  oled.print("pos: ");
  oled.print(pos);
  oled.display();


}
