#include <EEPROM.h>
/**
 * Print the Free memory along with a message in the Serial window
 *
 * Uses MemoryFree library - https://github.com/sudar/MemoryFree
 */
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

// Used for EEPROM management
void loadConfig() {
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2])
    for (unsigned int t=0; t<sizeof(WiFiConfig); t++)
      *((char*)&WiFiConfig + t) = EEPROM.read(CONFIG_START + t);
}

void saveConfig() {
  for (unsigned int t=0; t<sizeof(WiFiConfig); t++)
    EEPROM.write(CONFIG_START + t, *((char*)&WiFiConfig + t));
}
