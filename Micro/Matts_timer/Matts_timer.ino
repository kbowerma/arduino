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
  pinMode(13, OUTPUT);
  Serial.begin(9600);
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

