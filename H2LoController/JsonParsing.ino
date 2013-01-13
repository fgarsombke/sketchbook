struct Zone parseZone(String zoneJson) {
  // {id:3,zoneStatus:ON}
  struct Zone myZone;
  Serial.println("Parsing JSON Zone");
  String id = zoneJson.substring(zoneJson.indexOf("id") +3,zoneJson.indexOf(','));
  Serial.println("id:" + id);
  myZone.id = id.toInt();
  String zoneStatus = zoneJson.substring(zoneJson.indexOf("zoneStatus") +11, zoneJson.indexOf('}'));
  Serial.println("zoneStatus:" + zoneStatus);
  myZone.zoneStatus = zoneStatus;
  return myZone;
}

String getCommand(String topic) {
  Serial.println("getCommand.");
  String command = topic.substring(topic.lastIndexOf('/') +1,topic.length());
  Serial.println("command:" + command);
  return command;
}
