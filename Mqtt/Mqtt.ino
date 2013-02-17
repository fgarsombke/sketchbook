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
#include <WiFi.h>
#include <WiFiClient.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };

#define MQTT_SERVER "ec2-50-17-66-9.compute-1.amazonaws.com"
#define M2MIO_USERNAME   "device"
#define M2MIO_PASSWORD   "D2afrEXeSWumech4daSP"
#define M2MIO_DOMAIN     ""
#define M2MIO_DEVICE_ID "arduino-h2lo-deviceXXX"
char ssid[] = "nintendo";          //  your network SSID (name) 
char pass[] = "7ustESTenedr";   // your network password
int status = WL_IDLE_STATUS;

//char* jsonString = "hello world";
//char* jsonString = "{\"deviceId\":2,\"zoneDurations\":[{\"zoneNumber\":3,\"duration\":5000},{\"zoneNumber\":4,\"duration\":10000}],\"hour\":0,\"minute\":0}";
char* jsonString = "{\"d\":2,\"z\":1,\"s\":10}                    ";
//char* jsonString = "test";

// Callback function header
void callback(char* topic, byte* payload, unsigned int length);

//EthernetClient netClient;
WiFiClient netClient;
PubSubClient client(MQTT_SERVER, 1883, callback, netClient);

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
  
  //IPAddress ip(192,168,1, 177);
  //Ethernet.begin(mac, ip);
  
  initWiFi();
  // give the ethernet module time to boot up:
  delay(1000);
  if (client.connect(M2MIO_DEVICE_ID, M2MIO_USERNAME, M2MIO_PASSWORD)) {
    Serial.print(F("Connected to MQTT"));
    client.subscribe("arduinoOutTopic");
  } else {
    Serial.print(F("Not connected to MQTT"));
  }
}

void loop()
{
  static int count = 0;
  client.loop();
  boolean publishSuccess = client.publish("arduinoOutTopic", jsonString);
  Serial.print(F("Publish Success?"));
  Serial.println(publishSuccess);
  Serial.print(F("Still connected?"));
  Serial.println(client.connected());
  delay(1000);
  count++;
  Serial.print(F("count:")); 
  Serial.println(count);  
}

void initWiFi() {
  
  Serial.println("Attempting to connect to WPA network...");
  Serial.print("SSID: ");
  Serial.println(ssid);

  status = WiFi.begin(ssid, pass);
  if ( status != WL_CONNECTED) { 
    Serial.println("Couldn't get a wifi connection");
    while(true);
  } 
  else {
    Serial.print("Connected to wifi. My address:");
    IPAddress myAddress = WiFi.localIP();
    Serial.println(myAddress);
  }
}
