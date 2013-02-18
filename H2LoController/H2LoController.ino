/*
  H2LoController
 created 04 Jan 2012
 by Franz Garsombke
 */
#include <SPI.h>
#include <PubSubClient.h>
#include <aJSON.h>
#include <QueueArray.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <MemoryFree.h>

// MQTT
#define MQTT_SERVER "ec2-50-17-66-9.compute-1.amazonaws.com"
#define M2MIO_USERNAME   "device"
#define M2MIO_PASSWORD   "D2afrEXeSWumech4daSP"
#define M2MIO_DOMAIN     ""
#define M2MIO_DEVICE_ID "arduino-h2lo-devicezzz"

// Command codes
const int CODE_CMD_NO_ACTION = 0;
const int CODE_CMD_UPDATE_ZONE_STATUS = 1;
const int CODE_CMD_RUN_ZONE_SCHEDULE = 2;
const int ENSURE_CONNECTION_MILLIS = 30000;

const int NO_SCHEDULE_RUNNING = 0;

const int ZONE_STATUS_OFF = 0;
const int ZONE_STATUS_ON = 1;

// structure to hold JSON schedule
struct Schedule
{
  int zoneNumber;
  long duration;
  int zoneStatus;
};

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
int zones[] = {zone1, zone2, zone3, zone4, zone5, zone6, zone7, zone8};
int zoneCount = 8;
// Callback function header
void callback(char* topic, byte* payload, unsigned int length);
WiFiClient netClient;
PubSubClient pubSubClient(MQTT_SERVER, 1883, callback, netClient);
char ssid[] = "nintendo";          //  your network SSID (name) 
char pass[] = "7ustESTenedr";   // your network password
int status = WL_IDLE_STATUS;
int zoneId;
int currentCommand = CODE_CMD_NO_ACTION;
long devicePIN = 0;
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
  Serial.print(F("payload:"));
  String jsonResponse = String(message_buff);
  Serial.println(jsonResponse);
  currentCommand = getCurrentCommand(String(topic));
}

void setup() {
  // start serial port:
  Serial.begin(9600);
  //Set relay pins to output
  for (int i = 0; i < zoneCount; i++){
    pinMode(zones[i], OUTPUT);
  }
  // clear the zones
  for (int i = 0; i < zoneCount; i++){
    Serial.print(F("Clearing PIN:"));
    Serial.println(zones[i]);
    digitalWrite(zones[i], LOW);
  }
  devicePIN = 16807;
}

void loop() {
  if (millis() > connect_alarm) {
    ensure_connected(); 
  }
  // MQTT client loop processing
  pubSubClient.loop();
  delay(2000);
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

void runCurrentCommand() {
  struct Schedule schedule = parseZoneJson(message_buff);
  switch (currentCommand) {
    case CODE_CMD_UPDATE_ZONE_STATUS: // Turn zone ON/OFF
      changeZoneStatus(schedule);
    break;
    case CODE_CMD_RUN_ZONE_SCHEDULE: // Run a whole zone schedule
      // put the schedule on the stack for processing
      scheduleStack.push(schedule);
      Serial.print(F("stack size after push:"));
      Serial.println(scheduleStack.count());    
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
  Serial.println(F("changing zone status"));
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
  
  Serial.print(F("requesting run for zone#:"));
  Serial.println(zone);
  Serial.print(F("starting run on pin:"));
  Serial.println(pin);
  
  // turn on selected zone
  digitalWrite(pin, HIGH);
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
    }
  }
}

void endTimedRun(){
  // if ZONE == 0 nothing is running
  if(commandRunning[CR_ZONE_ID] == CODE_CMD_NO_ACTION) {
    return;
  }
  
  Serial.print(F("deactivating zone:"));
  Serial.println(commandRunning[CR_ZONE_ID]);
  
  // turn off the pin for the active zone
  digitalWrite(commandRunning[CR_PIN_ID], LOW);
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
  static int ensureConnectionCount = 0;
  ensureConnectionCount++;
  Serial.print(F("Free RAM:"));
  Serial.println(freeRam());
  Serial.print(F("Ensure connection count:"));
  Serial.println(ensureConnectionCount);
  connect_alarm = millis() + ENSURE_CONNECTION_MILLIS;
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
    if (wifi_connected) {
      mqtt_connect();
    }
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

void clearCommandRunning(){
  for (int i = 0; i < commandRunningLength; i++){
    commandRunning[i] = 0;
  }
}

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

int getCurrentCommand(String topic) {
  int command = topic.substring(topic.lastIndexOf('/') +1,topic.length()).toInt();
  /*
  Serial.print(F("command:"));
  Serial.println(command);
  */
  return command;
}


char* retrieveZoneStatusJson(int zoneNumber, int zoneStatus) {
  aJsonObject *root = aJson.createObject();
  aJson.addNumberToObject(root,"z", zoneNumber);
  aJson.addNumberToObject(root,"s", zoneStatus);
  char* json_string = aJson.print(root);
  // clean up the resources
  aJson.deleteItem(root);
  return json_string;
}

Schedule parseZoneJson(char *jsonString) {
  struct Schedule schedule;
  aJsonObject* root = aJson.parse(jsonString);
  if (root != NULL) {
    //Serial.println("Parsed successfully Root " );
    aJsonObject* zone = aJson.getObjectItem(root, "z"); 
    if (zone != NULL) {
      //Serial.println("Parsed successfully zone" );
      schedule.zoneNumber = zone->valueint;
      Serial.print(F("zone:"));
      Serial.println(schedule.zoneNumber);
    }
    aJsonObject* duration = aJson.getObjectItem(root, "d"); 
    if (duration != NULL) {
      //Serial.println("Parsed successfully duration" );
      schedule.duration = duration->valueint;
      Serial.print(F("duration:"));
      Serial.println(schedule.duration);
    }        
    aJsonObject* zoneStatus = aJson.getObjectItem(root, "s"); 
    if (zoneStatus != NULL) {
      //Serial.println("Parsed successfully duration" );
      schedule.zoneStatus = zoneStatus->valueint;
      Serial.print(F("zoneStatus:"));
      Serial.println(schedule.zoneStatus);
    }     
  }
  // This deletes the objects and all values referenced by it.  
  aJson.deleteItem(root);
  return schedule;
}
