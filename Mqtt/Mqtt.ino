/*
 Publishing in the callback 
 
  - connects to an MQTT server
  - subscribes to the topic "inTopic"
  - when a message is received, republishes it to "outTopic"
  
  This example shows how to publish messages within the
  callback function. The callback function header needs to
  be declared before the PubSubClient constructor and the 
  actual callback defined afterwards.
  This ensures the client reference in the callback function
  is valid.
  
*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };

#define MQTT_SERVER "192.168.1.13"
#define M2MIO_USERNAME   ""
#define M2MIO_PASSWORD   ""
#define M2MIO_DOMAIN     ""
#define M2MIO_DEVICE_ID "arduino-h2lo-device"

//char* jsonString = "hello world";
//char* jsonString = "{\"deviceId\":2,\"zoneDurations\":[{\"zoneNumber\":3,\"duration\":5000},{\"zoneNumber\":4,\"duration\":10000}],\"hour\":0,\"minute\":0}";
char* jsonString = "{\"deviceId\":2,\"zoneDurations\":[{\"zoneNumber\":3,\"duration\":5000}],\"hour\":0,\"minute\":0}";

// Callback function header
void callback(char* topic, byte* payload, unsigned int length);

EthernetClient ethClient;
PubSubClient client(MQTT_SERVER, 1883, callback, ethClient);

// Callback function
void callback(char* topic, byte* payload, unsigned int length) {
  //Serial.print(F("length:"));
  //Serial.println(length);
  char message_buff[length+1];
  Serial.print(F("topic:"));
  Serial.println(topic);  
  // In order to republish this payload, a copy must be made
  // as the orignal payload buffer will be overwritten whilst
  // constructing the PUBLISH packet.
  int i = 0;  
  // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  Serial.print(F("payload:"));
  String jsonResponse = String(message_buff);
  Serial.println(jsonResponse);
  
}

void setup() {
  // start serial port:
  Serial.begin(9600);
  // give the ethernet module time to boot up:
  delay(1000);
  IPAddress ip(192,168,1, 177);
  Ethernet.begin(mac, ip);
  if (client.connect("arduinoClient", "foobar", "foobarpassword")) {
    client.subscribe("arduinoOutTopic");
  }
}

void loop()
{
  static int count = 0;
  client.loop();
  client.publish("arduinoOutTopic", jsonString);
  delay(1000);
  count++;
  Serial.print(F("count:")); 
  Serial.println(count);  
}
