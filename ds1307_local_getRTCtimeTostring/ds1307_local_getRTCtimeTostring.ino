

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib

#include <Wire.h>
#include "RTClib.h"
#include <Streaming.h>

//includes from MegaEth_dtemp_v4
//#include <Streaming.h>
#include <SD.h>
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <util.h>
#include <EEPROM.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
//#include <Wire.h>
//#include <DS1307.h>   // I had to comment this out becuse of line RTC_DS1307 RTC;
//#include "RTClib.h"
#include <stdlib.h>




RTC_DS1307 RTC;

void setup () {
    Serial.begin(9600);
    Wire.begin();
    RTC.begin();

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
   // RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}

String timeString() {
    DateTime now = RTC.now();
  
    String myYear = String(now.year());
    String myMonth = String(now.month());
    String myDay = String(now.day());
    String myHour = String(now.hour());
    
    
    
    String myMinute = String(now.minute());
    if (now.minute() < 10) { 
      myMinute = String("0"+ String(now.minute()));
    }
    
    String mySecond = String(now.second());
    if (now.second() < 10) {
     mySecond = String("0"+ String(now.second()));
    } 
    
    String myDate = String(myMonth+"/"+myDay+"/"+myDay);
    String myTime = String(" "+myHour+":"+myMinute+":"+mySecond);
    
    String myDateTime = String(myDate + myTime);
    
    return myDateTime;
}




void loop () {

    Serial << endl << "myDateTime: " << timeString() << endl;
    
    delay(3000);
}
