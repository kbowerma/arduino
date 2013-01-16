#include <WProgram.h>
#include <Wire.h>
#include <DS1307.h>
//#include <Ethernet.h>
//#include <SPI.h>


//byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xDD };
//byte ip[] = { 192, 168, 11, 82 };
//Server server(80);

// written by mattt on the Arduino forum and modified by D. Sjunnesson
void setup()
{
Serial.begin(9600);
//Ethernet.begin(mac, ip);

  Serial.println("RTC_test");
  Serial.println("Ip address:  192.168.11.82");
  /*
  
RTC.stop();
RTC.set(DS1307_SEC,1);	//set the seconds
RTC.set(DS1307_MIN,12);	//set the minutes
RTC.set(DS1307_HR,11);	//set the hours
RTC.set(DS1307_DOW,3);	//set the day of the week
RTC.set(DS1307_DATE,19);	//set the date
RTC.set(DS1307_MTH,7);	//set the month
RTC.set(DS1307_YR,11);	//set the year
RTC.start();
*/
}


void loop()
{
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
delay(1000);
}
