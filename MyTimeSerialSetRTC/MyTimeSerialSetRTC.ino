/* 
 * TimeSerial.pde
 * example code illustrating Time library set through serial port messages.
 *
 * Messages consist of the letter T followed by ten digit time (as seconds since Jan 1 1970)
 * you can send the text on the next line using Serial Monitor to set the clock to noon Jan 1 2010
 T1262347200  
 date -j -v-4H -f "%a %b %d %T %Z %Y" "`date`" "+T%s" > /dev/tty.usbserial-A900ceXb  <- mac command off by 4 hours
 works fairly well but the seconds don't seems to set

 *
 * A Processing example sketch to automatically send the messages is inclided in the download
 */ 
 
#include <Time.h>  
#include <WProgram.h>
#include <Wire.h>
#include <DS1307.h>

#define TIME_MSG_LEN  11   // time sync to PC is HEADER followed by unix time_t as ten ascii digits
#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

int doonce = 0;
int ready = 0;

void setup()  {
  Serial.begin(9600);
  setSyncProvider( requestSync);  //set function to call when sync required
  Serial.println("Waiting for sync message");
  
}

void loop(){    
  if(Serial.available() ) 
  {
    processSyncMessage();
  }
  if(timeStatus()!= timeNotSet)   
  {
    digitalWrite(13,timeStatus() == timeSet); // on if synced, off if needs refresh  
    //digitalClockDisplay();  
  }
  while (doonce < 1 && ready == 1){
   RTC.stop();
    RTC.set(DS1307_SEC,second());	//set the seconds
    RTC.set(DS1307_MIN,minute());	//set the minutes
     Serial.println("the seconds are is");
     Serial.print(second());
     //delay(5000);
    RTC.set(DS1307_HR,hour());	//set the hours
   // RTC.set(DS1307_DOW,3);	//set the day of the week
    RTC.set(DS1307_DATE,day());	//set the date
    RTC.set(DS1307_MTH,month());	//set the month
    RTC.set(DS1307_YR,11);	//set the year
   RTC.start();
    
  doonce = 1;
  }
  
  if (doonce == 1 && ready == 1 ) {
  
  Serial.println("-------RTC Clock------------");
  Serial.print(RTC.get(DS1307_HR,true)); //read the hour and also update all the values by pushing in true
Serial.print(":");
Serial.print(RTC.get(DS1307_MIN,false));//read minutes without update (false)
Serial.print(":");
Serial.print(RTC.get(DS1307_SEC,false));//read seconds
Serial.print("	");	// some space for a more happy life
Serial.print(RTC.get(DS1307_MTH,false));//read date
Serial.print("/");
Serial.print(RTC.get(DS1307_DATE,false));//read month
Serial.print("/");
Serial.print(RTC.get(DS1307_YR,false)); //read year
Serial.println();
  }
  
  delay(1000);
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void processSyncMessage() {
  // if time sync available from serial port, update time and return true
  while(Serial.available() >=  TIME_MSG_LEN ){  // time message consists of a header and ten ascii digits
    char c = Serial.read() ; 
    Serial.print(c);  
    if( c == TIME_HEADER ) {       
      time_t pctime = 0;
      for(int i=0; i < TIME_MSG_LEN -1; i++){   
        c = Serial.read();          
        if( c >= '0' && c <= '9'){   
          pctime = (10 * pctime) + (c - '0') ; // convert digits to a number    
        }
      }   
      setTime(pctime);   // Sync Arduino clock to the time received on the serial port
      ready = 1;
    }  
  }
}

time_t requestSync()
{
  Serial.write(TIME_REQUEST);  
  return 0; // the time will be sent later in response to serial mesg
}

