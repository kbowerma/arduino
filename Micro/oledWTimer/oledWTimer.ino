/* 6.18.2015 Kyle Bowerman
* Matts Timer + OledSimple Text
*
*/
#include <Wire.h>  // Include Wire if you're using I2C
#include <SPI.h>  // Include SPI if you're using SPI
#include <SFE_MicroOLED.h>  // Include the SFE_MicroOLED library

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

// the setup function runs once when you press reset or power the board
void setup() {
  oled.begin();    // Initialize the OLED
  oled.clear(ALL); // Clear the display's internal memory
  oled.display();  // Display what's in the buffer (splashscreen)
  delay(1000);     // Delay 1000 ms
  oled.clear(PAGE); // Clear the buffer.
  Serial.begin(9600);
  delay(1000);
  print("microled_demo");
  int mySeconds = 0;
  int myMinutes = 0;
  int myHours = 0;
  int myDays = 0;

  pinMode(13, OUTPUT);
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

  timer(&printAnalogPins, 2000, timer1); // Call printAnalogPins every 2000ms
  timer(&hello, 10000, timer2); // Call hello every 10 seconds
  timer(&printDot, 200, timer3); // Just print dots every 200ms
  timer(&kyleText, 5000, timer4); // call kyleText every 5 seconds
  //timer(&kyleText2, 10000, timer2); //clear the page every 7 seconds
}

/*
void kyleClr()
{
  oled.clear(PAGE);
  oled.setCursor(0, 0);        // Set cursor to top-left
  oled.setFontType(0);         // Smallest font
  oled.print("Nothing ");
  oled.display();
}
*/

void kyleText2()
{
  // Demonstrate font 2. 10x16. Only numbers and '.' are defined.
  // This font looks like 7-segment displays.
  // Lets use this big-ish font to display readings from the
  // analog pins.
  for (int i=0; i<10; i++)
  {
    int mySeconds = (millis() % 60000)/1000;
    int myMinutes = (millis() % 3600000)/60000;  //3600000 milliseconds in an hour
    int myHour = (millis() % 86400000)/3600000; //86400000 miliisceconds in a day

    oled.clear(PAGE);            // Clear the display
    oled.setCursor(0, 0);        // Set cursor to top-left
    oled.setFontType(0);         // Smallest font
    oled.print("A3: ");          // Print "A0"
    //oled.setFontType(1);         // 7-segment font
    oled.print(analogRead(A3));  // Print a0 reading
    oled.setCursor(0, 12);       // Set cursor to top-middle-left
    //oled.setFontType(0);         // Repeat
    oled.print("A4: ");
    //oled.setFontType(1);
    oled.print(analogRead(A4));
    oled.setCursor(0, 24);
    //oled.setFontType(0);
    oled.print("A5: ");
    //oled.setFontType(1);
    oled.print(analogRead(A5));
    oled.setCursor(0,36);
    oled.print("T ");
    oled.print(myHour);
    oled.print(":");
    oled.print(myMinutes);
    oled.print(":");
    oled.print(mySeconds);
    oled.display();
    delay(100);
  }

}

void kyleText()
{
  // Demonstrate font 2. 10x16. Only numbers and '.' are defined.
  // This font looks like 7-segment displays.
  // Lets use this big-ish font to display readings from the
  // analog pins.
  for (int i=0; i<10; i++)
  {
    int mySeconds = (millis() % 60000)/1000;
    int myMinutes = (millis() % 3600000)/60000;  //3600000 milliseconds in an hour
    int myHour = (millis() % 86400000)/3600000; //86400000 miliisceconds in a day

    oled.clear(PAGE);            // Clear the display
    oled.setCursor(0, 0);        // Set cursor to top-left
    oled.setFontType(0);         // Smallest font
    oled.print("A0: ");          // Print "A0"
    //oled.setFontType(1);         // 7-segment font
    oled.print(analogRead(A0));  // Print a0 reading
    oled.setCursor(0, 12);       // Set cursor to top-middle-left
    //oled.setFontType(0);         // Repeat
    oled.print("A1: ");
    //oled.setFontType(1);
    oled.print(analogRead(A1));
    oled.setCursor(0, 24);
    //oled.setFontType(0);
    oled.print("A2: ");
    //oled.setFontType(1);
    oled.print(analogRead(A2));
    oled.setCursor(0,36);
    oled.print("T ");
    oled.print(myHour);
    oled.print(":");
    oled.print(myMinutes);
    oled.print(":");
    oled.print(mySeconds);
    oled.display();
    delay(100);
  }

}

void printAnalogPins(){
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
