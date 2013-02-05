#include <aJSON.h>

// function definitions
void parseJson(char *jsonString) ;

// Json string to parse
char jsonString[] = "{\"deviceId\":2,\"s\":[{\"z\":3,\"d\":5000},{\"z\":4,\"d\":10000}],\"hour\":0,\"minute\":0}";

// structure to hold JSON schedule
struct Schedule
{
  int zoneNumber;
  int duration;
};

Schedule schedule[8];

void setup() {
    Serial.begin(9600);
    Serial.println(jsonString);
    Serial.println("Starting to parse");
    parseJson(jsonString);
}

void parseJson(char *jsonString) 
{
    aJsonObject* root = aJson.parse(jsonString);

    if (root != NULL) {
        Serial.println("Parsed successfully Root " );
        aJsonObject* zoneDurations = aJson.getObjectItem(root, "s"); 
        if (zoneDurations != NULL) {
            Serial.println("Parsed successfully zoneDurations" );
            // determine array size
            int zoneDurationsSize = aJson.getArraySize(zoneDurations);
            Serial.print(F("zoneDurationsSize:"));
            Serial.println(zoneDurationsSize);
            // iterate over zones
            for (int i = 0; i < zoneDurationsSize ; i++) {
              aJsonObject* zoneDuration = aJson.getArrayItem(zoneDurations, i);               
              if (zoneDuration != NULL) {
                Serial.println("Parsed successfully zoneDuration " );
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

void loop() {
}
