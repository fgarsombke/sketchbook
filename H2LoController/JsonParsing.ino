void parseZone(void *workingZone, char* payloadArray) { 
  // 7,ON
  Zone* myZone = (Zone*)workingZone;
  char delims[] = ",";
  //Serial.print(F("b4 parse:"));
  //Serial.println(freeRam());
  char *tokens;
  tokens = strtok(payloadArray, delims);
  Serial.print(F("zoneNumber:"));  
  Serial.println(tokens);
  myZone->zoneNumber = atoi(tokens);
  tokens = strtok(NULL, delims);
  Serial.print(F("zoneStatus:"));  
  Serial.println(tokens);  
  myZone->zoneStatus = tokens;
  //Serial.print(F("after parse:"));
  //Serial.println(freeRam());
}

int getCurrentCommand(String topic) {
  int command = topic.substring(topic.lastIndexOf('/') +1,topic.length()).toInt();
  Serial.print(F("command:"));
  Serial.println(command);
  return command;
}

void parseZoneScheduleJson(char *jsonString) {
  aJsonObject* root = aJson.parse(jsonString);

  if (root != NULL) {
    Serial.println(F("Parsed successfully Root"));
    aJsonObject* zoneDurations = aJson.getObjectItem(root, "s"); 
    if (zoneDurations != NULL) {
      Serial.println(F("Parsed successfully zoneDurations"));
      // determine array size
      int zoneDurationsSize = aJson.getArraySize(zoneDurations);
      Serial.print(F("zoneDurationsSize:"));
      Serial.println(zoneDurationsSize);
      // iterate over zones
      for (int i = 0; i < zoneDurationsSize ; i++) {
        aJsonObject* zoneDuration = aJson.getArrayItem(zoneDurations, i);               
        if (zoneDuration != NULL) {
          Serial.println(F("Parsed successfully zoneDuration"));
          aJsonObject* zoneNumber = aJson.getObjectItem(zoneDuration, "z"); 
          if (zoneNumber != NULL) {
            schedule[i].zoneNumber = zoneNumber->valueint;
            Serial.print(F("zoneNumber:"));
            Serial.println(schedule[i].zoneNumber);
          }
          aJsonObject* duration = aJson.getObjectItem(zoneDuration, "d"); 
          if (duration != NULL) {
            schedule[i].duration = duration ->valueint;
            Serial.print(F("duration:"));
            Serial.println(schedule[i].duration);
          }
        }
      }
    }
  }
}
