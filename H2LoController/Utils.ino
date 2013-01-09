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

void clearCharArray(char array[], int length){
  for (int i=0; i < length; i++) {
    //if (array[i] == 0) {break; };
    array[i] = 0;
  }
}

void clearCommandRunning(){
  for (int i = 0; i < commandRunningLength; i++){
    commandRunning[i] = 0;
  }
}
