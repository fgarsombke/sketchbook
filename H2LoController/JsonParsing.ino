int getCurrentCommand(String topic) {
  int command = topic.substring(topic.lastIndexOf('/') +1,topic.length()).toInt();
  /*
  Serial.print(F("command:"));
  Serial.println(command);
  */
  return command;
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
      /*
      Serial.print(F("zone:"));
      Serial.println(schedule.zoneNumber);
      */
    }
    aJsonObject* duration = aJson.getObjectItem(root, "d"); 
    if (duration != NULL) {
      //Serial.println("Parsed successfully duration" );
      schedule.duration = duration->valueint;
      /*
      Serial.print(F("duration:"));
      Serial.println(schedule.duration);
      */
    }        
    aJsonObject* zoneStatus = aJson.getObjectItem(root, "s"); 
    if (zoneStatus != NULL) {
      //Serial.println("Parsed successfully duration" );
      schedule.zoneStatus = zoneStatus->valueint;
      /*
      Serial.print(F("zoneStatus:"));
      Serial.println(schedule.zoneStatus);
      */
    }     
  }
  // This deletes the objects and all values referenced by it.  
  aJson.deleteItem(root);
  return schedule;
}
