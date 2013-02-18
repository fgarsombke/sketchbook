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
int pinStart = 22;
// Zone
int zone1 = pinStart++;
int zone2 = pinStart++; 
int zone3 = pinStart++; 
int zone4 = pinStart++; 
int zone5 = pinStart++; 
int zone6 = pinStart++; 
int zone7 = pinStart++; 
int zone8 = pinStart;
int zoneCount = 8;
int zones[] = {zone1, zone2, zone3, zone4, zone5, zone6, zone7, zone8};

void setup()
{
  Serial.begin(9600);
  //Set relay pins to output
  for (int i = 0; i < zoneCount; i++){
    pinMode(zones[i], OUTPUT);
  }
}

void loop()
{
  //Set relay pins to output
  for (int i = 0; i < zoneCount; i++){
    Serial.print(F("Turning ON PIN:"));
    Serial.println(zones[i]);    
    digitalWrite(zones[i], HIGH);   // sets the LED on
    delay(1000);                  // waits for a second
    digitalWrite(zones[i], LOW);    // sets the LED off
  }
}
