#include <TrueRandom.h>

byte uuidNumber[16]; // UUIDs in binary form are 16 bytes long

void printHex(byte number) {
  int topDigit = number >> 4;
  int bottomDigit = number & 0x0f;
  // Print high hex digit
  Serial.print( "0123456789ABCDEF"[topDigit] );
  // Low hex digit
  Serial.print( "0123456789ABCDEF"[bottomDigit] );
}

void printUuid(byte* uuidNumber) {
  int i;
  for (i=0; i<16; i++) {
    if (i==4) Serial.print("-");
    if (i==6) Serial.print("-");
    if (i==8) Serial.print("-");
    if (i==10) Serial.print("-");
    printHex(uuidNumber[i]);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.print("Generating a random number.");
  // Generate a new UUID
  TrueRandom.uuid(uuidNumber);
  
  Serial.print("The UUID number is ");
  printUuid(uuidNumber);
  Serial.println();
}

void loop() {
}
