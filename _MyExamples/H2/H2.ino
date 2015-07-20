/* 6.18.2015 Kyle Bowerman
* Matts Timer + OledSimple Text
* 6.22 Adding encoder
* 6.23 Fixing encoder from _MyExamples/MyBasic
* 6.23 apporting trying interuppts
* 7.2 renamed G2 from NonInrpts-oldedWTimerEncoder
* 7.2 renamed H2 from G2 for fork to Adafruit Oled 128x32 I2C
*
*/
#include <Wire.h>  // Include Wire if you're using I2C
#include <SPI.h>  // Include SPI if you're using SPI
#include <SFE_MicroOLED.h>  // Include the SFE_MicroOLED library
#include <Encoder.h> // enocer libarary
//Adafruit Oled 128x32
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 9
Adafruit_SSD1306 display(OLED_RESET);

// MicroOLED Definition //
/*
#define PIN_RESET 9  // Connect RST to pin 9
#define PIN_DC    8  // Connect DC to pin 8
#define PIN_CS    10 // Connect CS to pin 10
#define DC_JUMPER 1
*/


#define COMMON_ANODE


// MicroOLED Object Declaration //
//MicroOLED oled(PIN_RESET, PIN_DC, PIN_CS); // SPI declaration
//MicroOLED oled(PIN_RESET, DC_JUMPER);    // I2C declaration

//Endocer declaration
Encoder myEnc(7, 6);

// Prototypes
void timer(void (*)(void), int, unsigned long *);

// Globals
String inputBuffer;
String inputLine;
unsigned long *timer1 = new unsigned long;
unsigned long *timer2 = new unsigned long;
unsigned long *timer3 = new unsigned long;
unsigned long *timer4 = new unsigned long;
unsigned long *timer5 = new unsigned long;
long oldPosition  = -999;
int newtime = 0;
int oldtime = 0;
int button = 4;
int bluepin = 11;
int greenpin = 10;
int redpin = 5;
int m_wet = 800;
int m_ok = 300;
int m_danger = 100;
int m_dry = 10;

// the setup function runs once when you press reset or power the board
void setup() {

  Serial.begin(9600);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  // init done

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();

  delay(1500);
  print("microled_demo");
  int mySeconds = 0;
  int myMinutes = 0;
  int myHours = 0;
  int myDays = 0;

  pinMode(13, OUTPUT);
  pinMode(button, INPUT);
  digitalWrite(button, HIGH); // internal pullup
  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(bluepin, OUTPUT);

  //Serial.begin(9600);
  print("\n\nStartup...\n\n");
  // Make sure the timers start at 0
  (*timer1) = 0;
  (*timer2) = 0;
  (*timer3) = 0;
  (*timer4) = 0;
  (*timer5) = 0;

}



// the loop function runs over and over again forever
void loop() {
  // Every time the user hits 'enter', run executeCommand against the input
  checkUserInput();
  if (inputLine != "") {
    executeCommand(inputLine);
  }

  //  timer(&printAnalogPins, 2000, timer1); // Call printAnalogPins every 2000ms
  timer(&hello, 10000, timer2); // Call hello every 10 seconds
  //timer(&printDot, 200, timer3); // Just print dots every 200ms
  //timer(&printAnalog, 5000, timer4); // call kyleText every 5 seconds
  //timer(&printMode, 10000, timer2); //clear the page every 7 seconds

   doDispatch();

  //  testDisplay();


}

void testDisplay() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Hello, world!");
  display.display();
  delay(2000);
  display.setCursor(0,8);
  display.println("goodbye, world!");
  display.display();
  delay(2000);
  display.setCursor(0,16);
   display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.println("line 3 inverted");
  display.display();
  delay(2000);
  display.setCursor(0,24);
   display.setTextColor(WHITE);
  display.println("line 4 white");
  display.display();
  delay(2000);
  // Clear the buffer.
  display.clearDisplay();
  display.display();
  delay(2000);
}

void doDispatch() {
  long newPosition = myEnc.read();

  if (newPosition != oldPosition) {
    oldtime = newtime;
    newtime = millis();
    oldPosition = newPosition;
    Serial.print("this call made from doDispatch ");
    Serial.println(newPosition);
    //printMode();
    if (newPosition % 4 == 0 ) {
      if (newtime - oldtime > 3) {
        //printHead(newPosition);  // lagged behind
        printPos();
        Serial.print("newPosition: ");
        Serial.println(newPosition);
      }
      else {
        Serial.print("timediff ");
        Serial.println(newtime - oldtime);
      }
    }
  }

  if (newPosition < 0 )  myEnc.write(0); // dont let the postion go below 0
  //if (newPosition  < 1 ) {closeDisplay(5);} // Close display
  if (newPosition == 4 )  printAnalog();
  if (newPosition == 8 )  digitalWrite(13, HIGH);
  if (newPosition != 8 )  digitalWrite(13, LOW);
  if (newPosition == 12 ) printTime();
  if (newPosition == 16 ) checkMoisture();
  if (newPosition == 16 ) setColor(255,0,0); //red
  if (newPosition == 20 ) setColor(0,255,0); //green
  if (newPosition == 24 ) setColor(0,0,255); //blue
  if (newPosition == 28 ) setColor(255, 255,0); //yellow
  if (newPosition == 32 ) setColor(80,0,80); //purple
  if (newPosition == 36 ) setColor(0,255,255); //aqua
  if (newPosition == 40 ) setColor(255,165,0); //orange
  if (newPosition == 44 ) setColor(255,0,0); //red
  if (newPosition < 16 || newPosition > 44 ) {
     setColor(0,0,0);
    }
}

void checkMoisture() {
  int m1 = analogRead(A1);
  int m2 = analogRead(A2);
  int driestValue = 0;
  display.clearDisplay();
  display.setCursor(0,12);
  if (m1 < m2 ) {
    driestValue = m1;
    display.print("m1 ");
    display.print(m1);
  } else {
    driestValue = m2;
    display.print("m2 ");
    display.print(m2);
  }
  display.display();
  // Set the color button
  if ( driestValue < m_dry ) setColor(255,0,0); //red
  if ( driestValue > m_dry && driestValue < m_danger ) setColor(255,165,0); //orange
  if ( driestValue > m_danger && driestValue < m_ok ) setColor(255,255,0); //yellow
  if ( driestValue > m_ok && driestValue < m_wet ) setColor(0,255,0); //green
  if ( driestValue > m_wet ) setColor(0,0,255); //blue
}


void setColor(int red, int green, int blue)
{
  #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  #endif
  analogWrite(redpin, red);
  analogWrite(greenpin, green);
  analogWrite(bluepin, blue);
}



void printPos() {
  long cPosition = myEnc.read();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);        // Set cursor to top-left
  display.print("imode: ");
  display.print(cPosition);
  display.display();
}
/*
void closeDisplay(int timeout) {
  Serial.print("closing display");
  oled.clear(ALL);
  oled.print("sleep in\n");
  oled.print(timeout);
  oled.display();
  delay(timeout * 1000 );
  oled.clear(ALL);
}
*/

void printTime() {

  int mySeconds = (millis() % 60000)/1000;
  int myMinutes = (millis() % 3600000)/60000;  //3600000 milliseconds in an hour
  int myHour = (millis() % 86400000)/3600000; //86400000 miliisceconds in a day
  display.clearDisplay();
  display.setCursor(0,24);
  display.print("T ");
  display.print(myHour);
  display.print(":");
  display.print(myMinutes);
  display.print(":");
  display.print(mySeconds);
  display.display();
}


void printAnalog() {
    int mySeconds = (millis() % 60000)/1000;
    int myMinutes = (millis() % 3600000)/60000;  //3600000 milliseconds in an hour
    int myHour = (millis() % 86400000)/3600000; //86400000 miliisceconds in a day

    display.clearDisplay();  // had to clear display sinc ethey are writing overeachother
    display.setCursor(0, 12);       // Set cursor to top-middle-left
    display.print("A1: ");
    display.print(analogRead(A1));
    //display.setCursor(64, 12);  // Was 0,24
    display.print(" A2: ");
    display.print(analogRead(A2));
    display.setCursor(0,24);
    display.print("T ");
    display.print(myHour);
    display.print(":");
    display.print(myMinutes);
    display.print(":");
    display.print(mySeconds);

    display.display();

}


void printAnalogPins() {
  print("Analog Readout\n");
  print("--------------\n");
  print("Pin A0: " + String(analogRead(0)) + "\n");
  print("Pin A1: " + String(analogRead(1)) + "\n");
  print("Pin A2: " + String(analogRead(2)) + "\n");
  print("Pin A3: " + String(analogRead(3)) + "\n");
  print("Pin A4: " + String(analogRead(4)) + "\n");
  print("Pin A5: " + String(analogRead(4)) + "\n");
  print("\n\n");
}

void hello(){
  print("Hello World!\n\n");
  Serial.println(myEnc.read());
}

void printDot(){
  print(".");
}

/*
* timer function, pass it a funciton pointer, a time in ms, and a pointer to a timer (unsigned long)
* and it will fire off the function every m milliseconds
*/
void timer(void (*f)(void), int m, unsigned long *t){
  if ((millis() - (*t)) > (m - 1)){
      f();
    (*t) += m;
  }
}

// Main area for processing commands
void executeCommand(String command){
  inputLine = ""; // Clear out the input line

  if (command == "good") {
    print("It *is* good!\n");
    return;
  }

  if (command == "a0"){
    print("Pin A0: " + String(analogRead(0)) + "\n");
    return;
  }

  if (command == "led on") {
    digitalWrite(13, HIGH);
    return;
  }

  if (command == "led off") {
    digitalWrite(13, LOW);
    return;
  }
  if (command == "e") {
    Serial.print("the encoder postion is ");
    Serial.println( myEnc.read());
    return;
  }

  // Just echo back the command if nothing else...
  print(command + "\n");
}

// Keeps building the user input into inputBuffer until the user hits enter then copy it to inputLine and return it
String checkUserInput(){
  if (Serial.available() > 0) {
    switch (Serial.peek())
    {
      case 13: // Git rid of carriage returns
        Serial.read();
        break;
      case 10:
        print((char)Serial.read());
        inputLine = inputBuffer;
        inputBuffer = "";
        return inputLine;
        break;
      default:
        bufferInput(Serial.read());
    }
  }
  return "";
}

// Adds user's keystrokes to the inputBuffer
String bufferInput(char c){
    inputBuffer += c;
    // Serial.write("Buffered:");
    // Serial.print((int)c);
    // Serial.write("\n");
    print(c);
    return inputBuffer;
}

// Easier printing for serial... (overloaded functions)
void print(char c){
  Serial.write(c);
}
void print(int c){
  Serial.print(c);
}
void print(String s){
  int length = s.length();
  char * buffer = (char*) malloc (length + 1);
  s.toCharArray(buffer, length + 1);
  Serial.print(buffer);
  free (buffer);
}
