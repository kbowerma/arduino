



/*
derived From
 Udp NTP Client
 
 Get the time from a Network Time Protocol (NTP) time server
 Demonstrates use of UDP sendPacket and ReceivePacket 
 For more on NTP time servers and the messages needed to communicate with them, 
 see http://en.wikipedia.org/wiki/Network_Time_Protocol
 
 created 4 Sep 2010 
 by Michael Margolis
 modified 17 Sep 2010
 by Tom Igoe
 
 This code is in the public domain.
********** there is a bug in the ds1307 library and it wont set the seconds
 */

#include <SPI.h>         
#include <Ethernet.h>
#include <Udp.h>
#include <Streaming.h>
#include <Wire.h> 
#include <WProgram.h>
#include <DS1307.h>


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {  
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 
  192,168,11,82 };


unsigned int localPort = 8888;      // local port to listen for UDP packets

byte timeServer[] = { 
  192, 43, 244, 18}; // time.nist.gov NTP server

const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 

// A UDP instance to let us send and receive packets over UDP
//UDP Udp;
 
 int rtc[7];

void setup() 
{
  // start Ethernet and UDP
  Ethernet.begin(mac,ip);
  Udp.begin(localPort);

  Serial.begin(9600);
}

void loop()
{
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  Serial.println("Sent NTP Packet");

    // wait to see if a reply is available
  delay(1000);  
  if ( Udp.available() ) {  
    Udp.readPacket(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;  
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);               

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;     
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;  
    // print Unix time:
    Serial.println(epoch);                               


    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    //Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    RTC.stop();
        // Set the static stuff
         RTC.set(DS1307_DOW,6);	//set the day of the week
         RTC.set(DS1307_DATE,12);	//set the date
         RTC.set(DS1307_MTH,8);	//set the month
         RTC.set(DS1307_YR,11);	//set the year
    int myhour = ((epoch - 18000)  % 86400L) / 3600;
    RTC.set(DS1307_HR,myhour);
    Serial << "my hour is: " << myhour << endl;
   // Serial.print(':');  
   /* if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    } */
    //Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
      int myminute = (epoch  % 3600) / 60;
       RTC.set(DS1307_MIN,myminute);
      Serial << "my minute: " << myminute << endl;
  /*  Serial.print(':'); 
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    } */
    // Serial.println(epoch %60); // print the second
       int mysecond = epoch %60;
      RTC.set(DS1307_SEC,33);
       Serial << "my second: " << mysecond << endl;
         
    RTC.start();
  }
  // wait ten seconds before asking for the time again
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
  delay(10000); 
}

// send an NTP request to the time server at the given address 
unsigned long sendNTPpacket(byte *address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49; 
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp: 		   
  Udp.sendPacket( packetBuffer,NTP_PACKET_SIZE,  address, 123); //NTP requests are to port 123
}
