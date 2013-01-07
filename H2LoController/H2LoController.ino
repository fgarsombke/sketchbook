/*
  H2LoController
 
 created 04 Jan 2012
 by Franz Garsombke
 
 */
#include "Structures.h"
#include <SPI.h>
#include <Ethernet.h>
#include <aJSON.h>
#include <HttpClient.h>
#include <b64.h>
#include <MemoryFree.h>
#include <PubSubClient.h>

char server[] = "h2lo-api.herokuapp.com";
// Path to download (this is the bit after the hostname in the URL
// that you want to download
const char path[] = "/service/device";
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;
// Number of milliseconds to wait without receiving any data before we give up
const int networkTimeout = 30*1000;
// Number of milliseconds to wait if no data is available before trying again
const int networkDelay = 1000;
String jsonResponse = "";

void setup() {
  // start serial port:
  Serial.begin(9600);
  // give the ethernet module time to boot up:
  delay(1000);
  // start the Ethernet connection using a fixed IP address and DNS server:
   // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");
  // print the Ethernet board/shield's IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  printFreeMemory("After Setup");  
}

void loop() {
  
  int err =0;
  
  EthernetClient c;
  // This HTTPClient library https://interactive-matter.eu/how-to/arduino-http-client-library 
  // looks a little better (response is returned as a FILE* stream which aJSON can parse
  // but the http.get command takes an physical IPAddress and not just a server name. WTF?
  HttpClient http(c);
  
  err = http.get(server, path);
  if (err == 0)
  {
    Serial.println("startedRequest ok");

    err = http.responseStatusCode();
    if (err >= 0)
    {
      Serial.print("Got status code: ");
      Serial.println(err);

      // Usually you'd check that the response code is 200 or a
      // similar "success" code (200-299) before carrying on,
      // but we'll print out whatever response we get

      err = http.skipResponseHeaders();
      if (err >= 0)
      {
        int bodyLen = http.contentLength();
        Serial.print("Content length is: ");
        Serial.println(bodyLen);
        Serial.println();
        Serial.println("Body returned follows:");

        // Now we've got to the body, so we can print it out
        unsigned long timeoutStart = millis();
        char c;
        // Whilst we haven't timed out & haven't reached the end of the body
        while ( (http.connected() || http.available()) && ((millis() - timeoutStart) < networkTimeout) )
        {
            if (http.available())
            {
                c = http.read();
                // Print out this character
                Serial.print(c);
                if (c != '[' && c != ']') {
                  jsonResponse += c;              
                }
                bodyLen--;
                // We read something, reset the timeout counter
                timeoutStart = millis();
            }
            else {
              // We haven't got any data, so let's pause to allow some to arrive
              delay(networkDelay);
            }
        }
      }
      else
      {
        Serial.print("Failed to skip response headers: ");
        Serial.println(err);
      }
    }
    else
    {    
      Serial.print("Getting response failed: ");
      Serial.println(err);
    }
  }
  else
  {
    Serial.print("Connect failed: ");
    Serial.println(err);
  }
  http.stop();
  Serial.println();
  Serial.print("jsonResponse:");
  Serial.println(jsonResponse);
  // We have the json response, let's start to parse it
  jsonResponse.trim();
  char* jsonString = (char*) malloc(sizeof(char)*(jsonResponse.length()+1));
  jsonResponse.toCharArray(jsonString, jsonResponse.length() + 1);
  printFreeMemory("Before parsing");  
  Device device = parseDevice(jsonString);
  if (true) {
    Serial.print("Successfully Parsed pin:");
    Serial.println(device.pin);
    Serial.print("Successfully Parsed id:");
    Serial.println(device.id);
  } else {
    Serial.println("There was some problem in parsing the JSON");
  }
  printFreeMemory("After parsing");  
  // And just stop, now that we've tried a download
  while(1);
}

