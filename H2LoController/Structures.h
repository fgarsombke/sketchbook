#include <SPI.h>

// MQTT
#define MQTT_SERVER "m2m.eclipse.org"
#define M2MIO_USERNAME   ""
#define M2MIO_PASSWORD   ""
#define M2MIO_DOMAIN     ""
#define M2MIO_DEVICE_ID "arduino-h2lo-device"

// used for EEPROM management
#define CONFIG_VERSION "ar1"
#define CONFIG_START 0
// Command codes
const int CODE_CMD_NO_ACTION = 0;
const int CODE_CMD_UPDATE_ZONE_STATUS = 1;
const int CODE_CMD_RUN_ZONE_SCHEDULE = 2;

const int NO_SCHEDULE_RUNNING = 0;

const int ZONE_STATUS_OFF = 0;
const int ZONE_STATUS_ON = 1;

struct StorageStruct {
  char version[4];
  char ssid[24];
  char pwd[16];
  byte addr[4];
  unsigned int id;
  long devicePIN;
} WiFiConfig = {
  CONFIG_VERSION,
  "NetworkConnectTo",
  "",
  {0, 0, 0, 0},
  0,
  0
};

// structure to hold JSON schedule
struct Schedule
{
  int zoneNumber;
  long duration;
  int zoneStatus;
};

// Zone
int zone1 = 2; //pin 2
int zone2 = 3; //pin 3
int zone3 = 4; //pin 4
int zone4 = 5; //pin 5
int zone5 = 6; //pin 6
int zone6 = 7; //pin 7
int zone7 = 8; //pin 8
int zone8 = 9; //pin 9
int zones[] = {zone1, zone2, zone3, zone4, zone5, zone6, zone7, zone8};
int zoneCount = 8;
