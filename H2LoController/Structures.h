#include <SPI.h>

// structure to hold JSON Device
typedef struct Device
{
  String pin;
  int id;
} Device;

// Command Running structure - for managing running commands
int commandRunningLength = 4;
unsigned long commandRunning[] = {0, // pin ID, 0 for none
                                  0, // run end time in milliseconds, 0 for none
                                  0, // zone ID, 0 for none
                                  0  // run start time in milliseconds, 0 for none
                                  };
