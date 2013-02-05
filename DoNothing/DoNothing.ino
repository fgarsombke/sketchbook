int zone1 = 2; //pin 2
int zone2 = 3; //pin 3
int zone3 = 4; //pin 4
int zone4 = 5; //pin 5
int zone5 = 6; //pin 6
int zone6 = 7; //pin 7
int zone7 = 8; //pin 8
int zone8 = 9; //pin 9

int zoneCount = 8;
int zones[] = {zone1, zone2, zone3, zone4, zone5, zone6, zone7, zone8};
#include <MemoryFree.h>

void setup()
{
  // start serial port:
  Serial.begin(9600);
  Serial.println(F("I am doing...nothing."));  
  //Set relay pins to output
  for (int i = 0; i < zoneCount; i++){
    pinMode(zones[i], OUTPUT);
  }
  Serial.print(F("Free:"));
  Serial.println(getFreeMemory());  
}

void loop() {
}
