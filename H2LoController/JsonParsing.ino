/**
 * Parse the JSON String. Uses aJson library
 * 
 * Refer to http://hardwarefun.com/tutorials/parsing-json-in-arduino
 */
struct Device parseDevice(char *jsonString) {
  struct Device myDevice;
  Serial.println("Parsing JSON");
  aJsonObject* root = aJson.parse(jsonString);
  if (root != NULL) {
    Serial.println("Parsed successfully root." );
    aJsonObject* pin = aJson.getObjectItem(root, "pin"); 
    if (pin != NULL) {
      myDevice.pin = pin->valuestring;
    }
    aJsonObject* id = aJson.getObjectItem(root, "id"); 
    if (id != NULL) {
      myDevice.id = id->valueint;
    }
  }
  return myDevice;
}
