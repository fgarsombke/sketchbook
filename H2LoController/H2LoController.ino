/*
  H2LoController
 created 04 Jan 2012
 by Franz Garsombke
 */
#include "Structures.h"
#include <SPI.h>
//#include <Ethernet.h>
#include <PubSubClient.h>
#include <MemoryFree.h>
#include <aJSON.h>
#include <QueueArray.h> // http://playground.arduino.cc/Code/QueueArray
#include <WiFi.h>
#include <WiFiClient.h>

// Enter a MAC address for your controller below.Newer Ethernet shields have a MAC address printed on a sticker on the shield??
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//EthernetClient netClient;
WiFiClient netClient;
PubSubClient pubSubClient(MQTT_SERVER, 1883, callback, netClient);
char ssid[] = "nintendo";          //  your network SSID (name) 
char pass[] = "7ustESTenedr";   // your network password
int status = WL_IDLE_STATUS;
int zoneId;
int currentCommand = CODE_CMD_NO_ACTION;
long devicePIN = 0;
// LED indicator pin variables
int ledIndicator = 13;
int ledState = LOW;
unsigned long ledFlashTimer = 0;       // timer for LED in milliseconds
unsigned long ledFlashInterval = 1000; // flash interval in milliseconds
int commandRunningLength = 4;
unsigned long commandRunning[] = {0, // pin ID, 0 for none
                                  0, // run end time in milliseconds, 0 for none
                                  0, // zone ID, 0 for none
                                  0  // run start time in milliseconds, 0 for none
                                  };
const int CR_PIN_ID     = 0;
const int CR_END_TIME   = 1;
const int CR_ZONE_ID    = 2;
const int CR_START_TIME = 3;
// create a stack of run schedules.
QueueArray <Schedule> scheduleStack;
char message_buff[200];
unsigned long connect_alarm; // time of next connection check;
boolean wifi_connected = false;

void setup() {
  // start serial port:
  Serial.begin(9600);
  // give the ethernet module time to boot up:
  delay(1000);
  Serial.print(F("Free:"));
  Serial.println(getFreeMemory());  
  initWiFi();
  //IPAddress ip(192,168,1, 177);
  //Ethernet.begin(mac, ip);
  // give the Ethernet shield a second to initialize:
  delay(2000);
  //Set relay pins to output
  for (int i = 0; i < zoneCount; i++){
    pinMode(zones[i], OUTPUT);
  }
  // clear the zones
  for (int i = 0; i < zoneCount; i++){
    digitalWrite(zones[i], LOW);
  }
  // load EEPROM information
  loadConfig();
  Serial.print(F("Device PIN:"));
  Serial.println(WiFiConfig.devicePIN);  
  /* uncomment out to reset device PIN
  WiFiConfig.devicePIN = 0;
  saveConfig();
  */
  // set the global devicePIN
  devicePIN = WiFiConfig.devicePIN;
  Serial.print(F("after:"));
  Serial.println(freeRam());
  // reset all connections
  ensure_connected();
}

void loop() {
  if (millis() > connect_alarm) {
    ensure_connected(); 
  }
  // MQTT client loop processing
  pubSubClient.loop();
  delay(2000);
  // Must be called from 'loop'. This will service all the events associated with the timer. - http://playground.arduino.cc/Code/Timer
  //timer.update();    
  // This action is received from callback. If there is a command value, execute on it
  if(currentCommand != CODE_CMD_NO_ACTION) {
    runCurrentCommand();
  }
  // are there any schedules on the stack that we need to run?
  runSchedules();
  // check on timed runs, shutdown expired runs
  checkTimedRun();
}

void runSchedules() {
  // run the next schedule off of the stack if there are more schedules and nothing is currently running
  if(!scheduleStack.isEmpty() && commandRunning[CR_END_TIME] == NO_SCHEDULE_RUNNING) {
    Schedule poppedSchedule = scheduleStack.pop();
    /*
    Serial.print(F("stack size after pop:"));
    Serial.println(scheduleStack.count());
    */
    // Turn the zone ON
    startTimedRun(poppedSchedule.zoneNumber, poppedSchedule.duration);    
  }
}

// handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {
  /*
  Serial.print(F("length:"));
  Serial.println(length);
  Serial.print(F("topic:"));
  Serial.println(topic);
  */
  int i = 0;  
  // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  Serial.println(getFreeMemory());
  /*
  Serial.print(F("payload:"));
  String jsonResponse = String(message_buff);
  Serial.println(jsonResponse);
  */
  currentCommand = getCurrentCommand(String(topic));
}

void runCurrentCommand() {
  struct Schedule schedule = parseZoneJson(message_buff);
  switch (currentCommand) {
    case CODE_CMD_UPDATE_ZONE_STATUS: // Turn zone ON/OFF
      changeZoneStatus(schedule);
    break;
    case CODE_CMD_RUN_ZONE_SCHEDULE: // Run a whole zone schedule
      // put the schedule on the stack for processing
      scheduleStack.push(schedule);
      /*
      Serial.print(F("stack size after push:"));
      Serial.println(scheduleStack.count());    
      */
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

void changeZoneStatus(struct Schedule schedule) {
  /*
   *
    Should we clear out the schedule stack if we get these manual controls?
   * 
   */
  //Serial.println(F("changing zone status"));
  if (schedule.zoneStatus == ZONE_STATUS_ON) {
    // start a timed run with zone and duration information
    startTimedRun(schedule.zoneNumber, schedule.duration);
  } else {
    endTimedRun();
  }
}

void publishZoneChangeMessage(int zoneNumber, int zoneStatus) {
  char publishTopic[50];
  char* pubMsg = retrieveZoneStatusJson(zoneNumber, zoneStatus);
  /*
  Serial.print(F("publish message:"));
  Serial.println(pubMsg);
  */
  String publishTopicStr = "h2lo/";
  publishTopicStr.concat("cloud/");
  publishTopicStr.concat(devicePIN);
  publishTopicStr.concat("/UPDATE_ZONE_STATUS");
  publishTopicStr.toCharArray(publishTopic, publishTopicStr.length()+1); 
  //Serial.print(F("publish topic:"));
  //Serial.println(publishTopic);
  pubSubClient.publish(publishTopic, pubMsg);
}

void startTimedRun(int zone, unsigned long seconds){
  // deactivate last zone before starting another
  endTimedRun();
  // select pin (shift object id to base zero):
  int pin = zones[zone - 1];
  /*
  Serial.print(F("requesting run for zone#:"));
  Serial.println(zone);
  Serial.print(F("starting run on pin:"));
  Serial.println(pin);
  */
  // turn on selected zone
  digitalWrite(pin, HIGH);
  // turn on the LED indicator light
  digitalWrite(ledIndicator, HIGH);  // set the LED on
  ledFlashTimer = millis() + 10000;   // set the timer
  ledState = HIGH;                   // set the state
  // set commandRunning parameters
  commandRunning[CR_ZONE_ID] = zone;
  commandRunning[CR_PIN_ID] = pin; //BUG?: int to unsigned long?
  commandRunning[CR_START_TIME] = millis();
  commandRunning[CR_END_TIME] = commandRunning[CR_START_TIME] + (seconds * 1000);
  // send a message to the cloud that a zone status has changed
  publishZoneChangeMessage(zone, ZONE_STATUS_ON);  
}
/*
  Check on timedRuns, stop runs on expiration time
*/
void checkTimedRun(){
  // check for running command
  if (commandRunning[CR_END_TIME] > 0){
    // a command is running, check for time end
    if (millis() >= commandRunning[CR_END_TIME]){
      // close valve: stop zone X
      endTimedRun();
    } else {
    // Flash the LED
    if (millis() >= ledFlashTimer){
      if (ledState == LOW){
        ledState = HIGH;
      } else {
        ledState = LOW;
      }
      // Change the LED state
      digitalWrite(ledIndicator, ledState);
      // reset the timer for the next state change
      ledFlashTimer = millis() + ledFlashInterval;
    }
   }
  }
}

void endTimedRun(){
  // if ZONE == 0 nothing is running
  if(commandRunning[CR_ZONE_ID] == CODE_CMD_NO_ACTION) {
    return;
  }
  /*
  Serial.print(F("deactivating zone:"));
  Serial.println(commandRunning[CR_ZONE_ID]);
  */
  // turn off the pin for the active zone
  digitalWrite(commandRunning[CR_PIN_ID], LOW);
  // turn off the LED indicator
  digitalWrite(ledIndicator, LOW);    // set the LED off
  ledState = LOW;                     // reset the state
  ledFlashTimer = 0;                  // reset the timer
  // send a message to the cloud that a zone status has changed
  publishZoneChangeMessage(commandRunning[CR_ZONE_ID], ZONE_STATUS_OFF);  
  // clear the commandRunning array
  clearCommandRunning();
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
    Serial.println(email); 
    String publishTopicStr = "h2lo/";
    long randNumber = random(500000);
    publishTopicStr.concat("cloud/");
    publishTopicStr.concat(randNumber);
    publishTopicStr.concat("/SUBSCRIBE");
    publishTopicStr.toCharArray(subscriptionPublishTopic, publishTopicStr.length()+1); 
    Serial.print(F("publish topic:"));
    Serial.println(subscriptionPublishTopic);
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

/*
 * method to make sure we have a connection to the Wifi and MQTT server
 *
 */
void ensure_connected() {
  static int failureCountMQTT = 0;
  static int failureCountWiFi = 0;
  Serial.println(F("Ensuring connection."));
  connect_alarm = millis() + 30000;
  if (!pubSubClient.connected()) {   
    Serial.print(F("failureCountMQTT:"));
    Serial.println(failureCountMQTT);
    failureCountMQTT++;
    if (!wifi_connected) {
      Serial.print(F("failureCountWiFi:"));
      Serial.println(failureCountWiFi);
      failureCountWiFi++;
      initWiFi();
    }
    mqtt_connect();
  } else {
    publishHeartbeat();
  }
}

void initWiFi() {
  Serial.println(F("Attempting to connect to WPA network..."));
  Serial.print(F("SSID:"));
  Serial.println(ssid);
  status = WiFi.begin(ssid, pass);
  if ( status != WL_CONNECTED) { 
    Serial.println(F("Couldn't get a wifi connection"));
    wifi_connected = false;
  } 
  else {
    Serial.print(F("Connected to wifi. My address:"));
    IPAddress myAddress = WiFi.localIP();
    Serial.println(myAddress);
    wifi_connected = true;
    delay(1000);
  }
}

void mqtt_connect() {
    Serial.println(F("Connecting to MQTT Broker..."));
    if (pubSubClient.connect(M2MIO_DEVICE_ID, M2MIO_USERNAME, M2MIO_PASSWORD)) {
      Serial.println(F("Connected to MQTT"));
      subscribeToTopic();
      // tell everyone we are alive!
      publishHeartbeat();
    } else {
      Serial.println(F("Failed connecting to MQTT"));
    }
}
