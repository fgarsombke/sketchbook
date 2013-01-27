struct Zone parseZone(String zoneJson) {
  // {"status":"ON", "zoneNumber":7}
  struct Zone myZone;
  String zoneStatus = zoneJson.substring(zoneJson.indexOf("status") +9, zoneJson.indexOf(',')-1);
  Serial.println("zoneStatus:" + zoneStatus);
  myZone.zoneStatus = zoneStatus;
  String zoneNumber = zoneJson.substring(zoneJson.indexOf("zoneNumber") +12,zoneJson.indexOf('}'));
  Serial.println("zoneNumber:" + zoneNumber);
  myZone.zoneNumber = zoneNumber.toInt();
  return myZone;
}

String getCommand(String topic) {
  Serial.println("getCommand.");
  String command = topic.substring(topic.lastIndexOf('/') +1,topic.length());
  Serial.println("command:" + command);
  return command;
}
