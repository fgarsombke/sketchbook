/*
  H2LoController
 created 04 Jan 2012
 by Franz Garsombke
 */
#include "Structures.h"
#include <SPI.h>
#include <Ethernet.h>
#include <aJSON.h>
#include <MemoryFree.h>
#include <PubSubClient.h>

#define MQTT_SERVER "m2m.eclipse.org"
#define M2MIO_USERNAME   ""
#define M2MIO_PASSWORD   ""
#define M2MIO_PASSWORD   ""
#define M2MIO_DOMAIN     ""
#define M2MIO_DEVICE_ID "arduino-h2lo-device"
#define MQTT_KEEPALIVE 5
char jsonDeviceString[] = "{\"id\":1,\"pin\":\"12345\"}";

// Enter a MAC address for your controller below.Newer Ethernet shields have a MAC address printed on a sticker on the shield??
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetClient ethernetClient;
String jsonResponse = "";
char clientID[50];
char topic[50];
char msg[50];
char message_buff[100];
PubSubClient pubSubClient(MQTT_SERVER, 1883, callback, ethernetClient);

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
  Serial.println("connecting to ethernet...");
  // print the Ethernet board/shield's IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  Serial.println("connected to Ethernet...");
  printFreeMemory("After Setup"); 
 
  String topicStr = "arduino/";
  topicStr.concat("h2lo");
  topicStr.toCharArray(topic, topicStr.length()+1);
  Serial.print("topic:");
  Serial.println(topic);    
  Serial.println("connecting pub sub client");
  pubSubClient.connect(M2MIO_DEVICE_ID);
  pubSubClient.subscribe(topic);      
}

void loop() {
   if (!pubSubClient.connected()) {
      // clientID, username, MD5 encoded password
      Serial.println("re-connecting pub sub client");
      pubSubClient.connect(M2MIO_DEVICE_ID);
      pubSubClient.subscribe(topic);      
    }
    // MQTT client loop processing
    pubSubClient.loop();
    pubSubClient.publish(topic, jsonDeviceString);
    delay(5000);    
}

// handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Message arrived:  topic: " + String(topic));
  Serial.println("Length: " + String(length,DEC));
  int i = 0;  
  // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  String msgString = String(message_buff);
  Serial.print("Payload:");
  Serial.print(msgString);
  Serial.println(":");
  printFreeMemory("memory before parsing"); 
  // Run out of memory after 4 parses!!! 
  /*
  Device device = parseDevice(message_buff);
  Serial.print("Successfully Parsed pin:");
  Serial.println(device.pin);
  Serial.print("Successfully Parsed id:");
  Serial.println(device.id);  
  printFreeMemory("memory after parsing");  
  */
}
