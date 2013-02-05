/*
  H2LoController
 created 04 Jan 2012
 by Franz Garsombke
 */
#include "Structures.h"
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <MemoryFree.h>
#include <Event.h>
#include <Timer.h>
#include <aJSON.h>

// Enter a MAC address for your controller below.Newer Ethernet shields have a MAC address printed on a sticker on the shield??
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
Timer timer;
EthernetClient ethernetClient;
EthernetServer server(80);  
PubSubClient pubSubClient(MQTT_SERVER, 1883, callback, ethernetClient);
int zoneId;
int currentCommand = CODE_CMD_NO_ACTION;
Zone* workingZone = new Zone();
long devicePIN = 0;
// store all of the schedules in this array
Schedule schedule[8];
char message_buff[200];

void setup() {
  // start serial port:
  Serial.begin(9600);
  // give the ethernet module time to boot up:
  delay(1000);
  Serial.print(F("Free:"));
  Serial.println(getFreeMemory());  
  IPAddress ip(192,168,1, 177);
  Ethernet.begin(mac, ip);
  // give the Ethernet shield a second to initialize:
  delay(2000);
  //Set relay pins to output
  for (int i = 0; i < zoneCount; i++){
    pinMode(zones[i], OUTPUT);
  }
  Serial.println(F("mqtt initial connect"));
  pubSubClient.connect(M2MIO_DEVICE_ID);

  // load EEPROM information
  loadConfig();
  
  /* uncomment out to reset device PIN
  WiFiConfig.devicePIN = 0;
  saveConfig();
  */
  // determine if we use our webserver for device intial configuration or not
  if(WiFiConfig.devicePIN > 0) {
    // set the global devicePIN
    devicePIN = WiFiConfig.devicePIN;
    subscribeToTopic();
    // tell everyone we are alive!
    publishHeartbeat();
  } else {
    // Start the weberver
    //Serial.println("start web server");
    server.begin();    
  } 
  // Run the 'callback' every 'period' milliseconds.
  int publishHeartbeatEvent = timer.every(60000, publishHeartbeat);
  Serial.print(F("after:"));
  Serial.println(freeRam());
}

void loop() {
  if (!pubSubClient.connected()) {
    Serial.println(F("mqtt re-connecting"));
    pubSubClient.connect(M2MIO_DEVICE_ID);
    if(devicePIN > 0) {
      subscribeToTopic(); 
    }    
  }
  // MQTT client loop processing
  pubSubClient.loop();
  delay(1000);
  // determine if we use our webserver for device intial configuration or not
  if(devicePIN == 0) {
    // device has not been configured yet
    consumeHttpRequest();
  }
  // Must be called from 'loop'. This will service all the events associated with the timer. - http://playground.arduino.cc/Code/Timer
  if(devicePIN > 0) {
    timer.update();    
  }
  
  // This action is received from callback. If there is a command value, execute on it
  if(currentCommand != CODE_CMD_NO_ACTION) {
    runCurrentCommand();
  }    
}

// handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print(F("length:"));
  Serial.println(length);
  Serial.print(F("topic:"));
  Serial.println(topic);
  int i = 0;  
  // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  Serial.println(getFreeMemory());  
  Serial.print(F("payload:"));
  String jsonResponse = String(message_buff);
  Serial.println(jsonResponse);
  currentCommand = getCurrentCommand(String(topic));
}

void runCurrentCommand() {
  switch (currentCommand) {
    case CODE_CMD_UPDATE_ZONE_STATUS: // Turn zone ON/OFF
      parseZone(workingZone, message_buff);
      changeZoneStatus(workingZone);
    break;
    case CODE_CMD_RUN_ZONE_SCHEDULE: // Run a whole zone schedule
      parseZoneScheduleJson(message_buff);
    break;
    default:
      Serial.print(F("Unsupported command."));
    break;
  }
  // reset current command since we have already executed this one
  currentCommand = CODE_CMD_NO_ACTION;
  // clear out the message buffer
  memset(message_buff, 0, sizeof message_buff);
}

void changeZoneStatus(void* workingZone) {
  Zone* myZone = (Zone*)workingZone;  
  char pubMsg[50];
  char publishTopic[50];
  Serial.println(F("changing zone status"));
  if (myZone->zoneStatus.equals("ON")) {
    digitalWrite(myZone->zoneNumber +1, HIGH);
  } else {
    digitalWrite(myZone->zoneNumber +1, LOW);
  }
  //{"zoneNumber":7,"status":"ON"}
  String pubMsgStr = ("{\"zoneNumber\":");
  pubMsgStr.concat(myZone->zoneNumber);
  pubMsgStr.concat(",\"status\":\"");
  pubMsgStr.concat(myZone->zoneStatus);
  pubMsgStr.concat("\"}");
  pubMsgStr.toCharArray(pubMsg, pubMsgStr.length()+1);
  Serial.print(F("publish message:"));
  //Serial.println(pubMsg);
  String publishTopicStr = "h2lo/";
  publishTopicStr.concat("cloud/");
  publishTopicStr.concat(devicePIN);
  publishTopicStr.concat("/UPDATE_ZONE_STATUS");
  publishTopicStr.toCharArray(publishTopic, publishTopicStr.length()+1); 
  Serial.print(F("publish topic:"));
  //Serial.println(publishTopic);
  pubSubClient.publish(publishTopic, pubMsg);
}

void publishHeartbeat() {
  char publishTopic[50];
  char pubMsg[] = "ACTIVE";
  String publishTopicStr = "h2lo/";
  publishTopicStr.concat("cloud/");
  publishTopicStr.concat(devicePIN);
  publishTopicStr.concat("/HEARTBEAT");
  publishTopicStr.toCharArray(publishTopic, publishTopicStr.length()+1); 
  Serial.print(F("publish topic:"));
  Serial.println(publishTopic);
  pubSubClient.publish(publishTopic, pubMsg);  
}

void subscribe(char* email) {
  if(strlen(email) > 0) {
    char subscriptionPublishTopic[50];
    Serial.print(F("publish message:"));
    //Serial.println(email); 
    String publishTopicStr = "h2lo/";
    long randNumber = random(500000);
    publishTopicStr.concat("cloud/");
    publishTopicStr.concat(randNumber);
    publishTopicStr.concat("/SUBSCRIBE");
    publishTopicStr.toCharArray(subscriptionPublishTopic, publishTopicStr.length()+1); 
    Serial.print(F("publish topic:"));
    //Serial.println(publishTopic);
    pubSubClient.publish(subscriptionPublishTopic, email);

    // persist the device id    
    WiFiConfig.devicePIN = randNumber;
    saveConfig();
    // set the global PIN
    devicePIN = WiFiConfig.devicePIN;

    subscribeToTopic();
    
    // Now we clear our array
    memset( email, 0, sizeof(email) );    
  }
}

void subscribeToTopic() {
  char subscribeTopic[30];  
  String subscribeTopicStr = "h2lo/";
  subscribeTopicStr.concat("device/");
  subscribeTopicStr.concat(devicePIN);
  subscribeTopicStr.concat("/+");
  subscribeTopicStr.toCharArray(subscribeTopic, subscribeTopicStr.length()+1);
  Serial.print(F("subscribe:"));
  Serial.println(subscribeTopic);
  pubSubClient.subscribe(subscribeTopic);  
}

void consumeHttpRequest() {
  char email_buff[50];
  // Easy test:
  // curl -X POST -d "email=xxx@xxx.com" http://192.168.1.177
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println(F("new client"));
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
    delay(100);
    // close the connection:
    client.stop();
    if(email_buff > 0) {
      // subscribe the device
      subscribe(email_buff);
      // Now we clear our array
      memset(email_buff, 0, sizeof(email_buff) );      
    }  
    Serial.println(F("client disonnected"));
  }
}
