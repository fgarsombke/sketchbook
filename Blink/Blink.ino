/* Blinking LED
 * ------------
 *
 * turns on and off a light emitting diode(LED) connected to a digital  
 * pin, in intervals of 2 seconds. Ideally we use pin 13 on the Arduino 
 * board because it an LED built-in
 *
 * Created 1 June 2005
 * copyleft 2005 DojoDave <http://www.0j0.org>
 * http://arduino.berlios.de
 *
 * based on an orginal by H. Barragan for the Wiring i/o board
 */
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

void setup()
{
  //Set relay pins to output
  for (int i = 0; i < zoneCount; i++){
    pinMode(zones[i], OUTPUT);
  }
}

void loop()
{
  //Set relay pins to output
  for (int i = 0; i < zoneCount; i++){
    digitalWrite(zones[i], HIGH);   // sets the LED on
    delay(500);                  // waits for a second
    digitalWrite(zones[i], LOW);    // sets the LED off
  }
}
