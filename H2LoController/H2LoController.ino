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
//#include <TrueRandom.h>

// Server Commands
const String CMD_UPDATE_ZONE_STATUS = "UPDATE_ZONE_STATUS";
// Enter a MAC address for your controller below.Newer Ethernet shields have a MAC address printed on a sticker on the shield??
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetClient ethernetClient;
EthernetServer server(80);  
String jsonResponse = "";
char email_buff[50];
PubSubClient pubSubClient(MQTT_SERVER, 1883, callback, ethernetClient);
int zoneId;
String currentCommand;
static const struct Zone EmptyStruct;
//byte uuidNumber[16]; // UUIDs in binary form are 16 bytes long

void setup() {
  // start serial port:
  Serial.begin(9600);
  // give the ethernet module time to boot up:
  delay(1000);
  printFreeMemory("initial memory");
  IPAddress ip(192,168,1, 177);
  Ethernet.begin(mac, ip);
  // give the Ethernet shield a second to initialize:
  delay(2000);
  // print the Ethernet board/shield's IP address:
  Serial.print("My IP address:");
  Serial.println(Ethernet.localIP());
  printFreeMemory("After Setup memory"); 
  //Set relay pins to output
  for (int i = 0; i < zoneCount; i++){
    pinMode(zones[i], OUTPUT);
  }
  Serial.println("connecting pub sub client");
  pubSubClient.connect(M2MIO_DEVICE_ID);

  // load EEPROM information
  loadConfig();
  Serial.print("devicePIN:");
  Serial.println(WiFiConfig.devicePIN);
  
  /*
  // Generate a new UUID
  TrueRandom.uuid(uuidNumber);
  Serial.print("The UUID number is:");
  printUuid(uuidNumber);
  Serial.println();  
  */
  // determine if we use our webserver for device intial configuration or not
  if(WiFiConfig.devicePIN > 0) {
    subscribeToTopic();
  } else {
    // Start the weberver
    server.begin();    
  }  
}

void loop() {
  if (!pubSubClient.connected()) {
    Serial.println("re-connecting pub sub client");
    pubSubClient.connect(M2MIO_DEVICE_ID);
    if(WiFiConfig.devicePIN > 0) {
      subscribeToTopic(); 
    }    
  }
  // MQTT client loop processing
  pubSubClient.loop();
  delay(2000);
  //Serial.print("jsonResponse:");
  //Serial.println(jsonResponse);
  if(jsonResponse != NULL) {
    Serial.println("Do Something!");
    if (currentCommand.compareTo(CMD_UPDATE_ZONE_STATUS) == 0) {
      Zone zone = parseZone(jsonResponse);
      changeZoneStatus(zone);
      zone = EmptyStruct;
    }
    jsonResponse = NULL;
  }
  // determine if we use our webserver for device intial configuration or not
  if(WiFiConfig.devicePIN = 0) {
    // device has not been configured yet
    consumeHttpRequest();
    subscribe(email_buff);    
  }
}

// handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived:topic:");
  Serial.println(String(topic));
  Serial.println("Length:" + String(length,DEC));
  char message_buff[50];  
  int i = 0;  
  // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  printFreeMemory("memory before print buffer");
  jsonResponse = String(message_buff);
  currentCommand = getCommand(String(topic));
  Serial.print("current command:");
  Serial.println(currentCommand);
  printFreeMemory("memory after buffer");
}

void changeZoneStatus(Zone zone) {
  if (zone.zoneStatus.equals("ON")) {
    digitalWrite(zone.id +1, HIGH);
  } else {
    digitalWrite(zone.id +1, LOW);
  }
  //{"id":7,"status":"ON"}
  char pubMsg[50];
  char publishTopic[50];
  String pubMsgStr = ("{\"id\":");
  pubMsgStr.concat(zone.id);
  pubMsgStr.concat(",\"status\":\"");
  pubMsgStr.concat(zone.zoneStatus);
  pubMsgStr.concat("\"}");
  pubMsgStr.toCharArray(pubMsg, pubMsgStr.length()+1);
  Serial.print("publish message:");
  Serial.println(pubMsg);
  String publishTopicStr = "h2lo/";
  publishTopicStr.concat("cloud/");
  loadConfig();
  publishTopicStr.concat(WiFiConfig.devicePIN);
  publishTopicStr.concat("/UPDATE_ZONE_STATUS");
  publishTopicStr.toCharArray(publishTopic, publishTopicStr.length()+1); 
  Serial.print("publish topic:");
  Serial.println(publishTopic);
  pubSubClient.publish(publishTopic, pubMsg);
}

void subscribe(char* email) {
  if(strlen(email) > 0) {
    Serial.print("publish message:");
    Serial.println(email); 
    char publishTopic[30]; 
    String publishTopicStr = "h2lo/";
    long randNumber = random(500000);
    publishTopicStr.concat("cloud/");
    publishTopicStr.concat(randNumber);
    publishTopicStr.concat("/SUBSCRIBE");
    publishTopicStr.toCharArray(publishTopic, publishTopicStr.length()+1); 
    Serial.print("publish topic:");
    Serial.println(publishTopic);
    pubSubClient.publish(publishTopic, email);

    // persist the device id    
    WiFiConfig.devicePIN = randNumber;
    saveConfig();

    subscribeToTopic();
    
    // Now we clear our array
    memset( email, 0, sizeof(email) );    
  }
}

void subscribeToTopic() {
  loadConfig();
  char subscribeTopic[30];
  String subscribeTopicStr = "h2lo/";
  subscribeTopicStr.concat("device/");
  subscribeTopicStr.concat(WiFiConfig.devicePIN);
  subscribeTopicStr.concat("/+");
  subscribeTopicStr.toCharArray(subscribeTopic, subscribeTopicStr.length()+1);
  Serial.print("subscribeTopic:");
  Serial.println(subscribeTopic);
  pubSubClient.subscribe(subscribeTopic);  
}

void consumeHttpRequest() {
  // Easy test:
  // curl -X POST -d "email=xxx@xxx.com" http://192.168.1.177
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // this is to retrieve POST data
          int i = 0;  
          // create character buffer with ending null terminator (string)          
          while(client.available()) {
            char t = client.read();
            Serial.print(t);
            email_buff[i] = t;
            i++;
          }
          email_buff[i] = '\0';  
          Serial.println();
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connnection: close");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<form method=\"POST\">");
          client.println("Email: <input type=\"text\" name=\"email\"><br>");
          client.println("<input type=\"submit\" value=\"Submit\">");
          client.println("</form>");
          client.println("");       
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}
