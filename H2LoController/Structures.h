#include <SPI.h>

// structure to hold JSON Device
typedef struct Device
{
  String pin;
  int id;
} Device;

// structure to hold JSON Zone
typedef struct Zone
{
  String zoneStatus;
  int id;
} Zone;

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

// MQTT
#define MQTT_SERVER "broker.mqttdashboard.com"
#define M2MIO_USERNAME   ""
#define M2MIO_PASSWORD   ""
#define M2MIO_DOMAIN     ""
#define M2MIO_DEVICE_ID "arduino-h2lo-device"
#define MQTT_KEEPALIVE 5




