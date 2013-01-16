#include <WProgram.h>
#include <Wire.h>
#include <DS1307.h> 

int rtc[7];

void setup()
{
  Serial.begin(9600);
  /*
  RTC.stop();
  RTC.set(DS1307_SEC,1);
  RTC.set(DS1307_MIN,23);
  RTC.set(DS1307_HR,12);
  RTC.set(DS1307_DOW,1);
  RTC.set(DS1307_DATE,1);
  RTC.set(DS1307_MTH,10);
  RTC.set(DS1307_YR,7);
  RTC.start();
  */
}

void loop()
{
  RTC.get(rtc,true);

  for(int i=0; i<7; i++)
  {
    Serial.print(rtc[i]);
    Serial.print(" ");
  }
  Serial.println();

  delay(1000);

}
 
