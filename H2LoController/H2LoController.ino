/*
  H2LoController
 created 04 Jan 2012
 by Franz Garsombke
 */
#include "Structures.h"
#include <SPI.h>
#include <Ethernet.h>
#include <MemoryFree.h>
#include <PubSubClient.h>

int zone1 = 2; //pin 2
int zone2 = 3; //pin 3
int zone3 = 4; //pin 4
int zone4 = 5; //pin 5
int zone5 = 6; //pin 6
int zone6 = 7; //pin 7
int zone7 = 8; //pin 8
int zone8 = 9; //pin 9

// Server Commands
const String CMD_UPDATE_ZONE_STATUS = "updateZoneStatus";
int zones[] = {zone1, zone2, zone3, zone4, zone5, zone6, zone7, zone8};
int zoneCount = 8;
#define MQTT_SERVER "m2m.eclipse.org"
#define M2MIO_USERNAME   ""
#define M2MIO_PASSWORD   ""
#define M2MIO_DOMAIN     ""
#define M2MIO_DEVICE_ID "arduino-h2lo-device"
#define MQTT_KEEPALIVE 5
char jsonDeviceString[] = "{\"id\":\"1\",\"zoneStatus\":\"12345\"}";
// Enter a MAC address for your controller below.Newer Ethernet shields have a MAC address printed on a sticker on the shield??
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetClient ethernetClient;
String jsonResponse = "";
char subscribeTopic[50];
char publishTopic[50];
char msg[50];
char message_buff[100];
PubSubClient pubSubClient(MQTT_SERVER, 1883, callback, ethernetClient);
int zoneId;
String currentCommand;
static const struct Zone EmptyStruct;

void setup() {
  // start serial port:
  Serial.begin(9600);
  // give the ethernet module time to boot up:
  delay(1000);
  printFreeMemory("initial memory");
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
  printFreeMemory("After Setup memory"); 
  String subscribeTopicStr = "arduino/";
  subscribeTopicStr.concat("h2lo/device/3/+");
  subscribeTopicStr.toCharArray(subscribeTopic, subscribeTopicStr.length()+1);
  Serial.print("subscribeTopic:");
  Serial.println(subscribeTopic);    
  Serial.println("connecting pub sub client");
  pubSubClient.connect(M2MIO_DEVICE_ID);
  pubSubClient.subscribe(subscribeTopic);  
  //Set relay pins to output
  for (int i = 0; i < zoneCount; i++){
    pinMode(zones[i], OUTPUT);
  }
}

void loop() {
  if (!pubSubClient.connected()) {
    Serial.println("re-connecting pub sub client");
    pubSubClient.connect(M2MIO_DEVICE_ID);
    pubSubClient.subscribe(subscribeTopic);      
  }
  // MQTT client loop processing
  pubSubClient.loop();
  delay(1000);
  Serial.print("jsonResponse:");
  Serial.println(jsonResponse);
  if(jsonResponse != NULL) {
    Serial.println("Do Something!");
    if (currentCommand.compareTo(CMD_UPDATE_ZONE_STATUS) == 0) {
      Zone zone = parseZone(jsonResponse);
      changeZoneStatus(zone);
      zone = EmptyStruct;
    }
    jsonResponse = NULL;
  }
}

// handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived:topic:");
  Serial.println(String(topic));
  Serial.println("Length:" + String(length,DEC));
  int i = 0;  
  // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  printFreeMemory("memory before print buffer");
  jsonResponse = String(message_buff);
  currentCommand = getCommand(String(topic));
  printFreeMemory("memory after buffer");
  pubSubClient.publish("arduino/h2lo/api/1", "arduino status has changed");
}

void changeZoneStatus(Zone zone) {
  if (zone.zoneStatus.equals("ON")) {
    digitalWrite(zone.id +1, HIGH);
  } else {
    digitalWrite(zone.id +1, LOW);
  } 
}
