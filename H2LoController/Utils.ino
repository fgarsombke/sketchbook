#include <MemoryFree.h>
/**
 * Print the Free memory along with a message in the Serial window
 *
 * Uses MemoryFree library - https://github.com/sudar/MemoryFree
 */
void printFreeMemory(char* message) {
 Serial.print(message);
 Serial.print(":\t");
 Serial.println(getFreeMemory());
}
