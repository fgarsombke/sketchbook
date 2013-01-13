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

