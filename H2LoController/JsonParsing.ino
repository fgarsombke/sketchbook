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
  //Serial.println(command);
  return command;
}
