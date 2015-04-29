
/*
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
*/
#include <stdlib.h>
//#include <Ethernet.h>

#include <SPI.h>
#include <Ethernet.h>

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  Serial.print("this is a test");
  delay(2000);

}
