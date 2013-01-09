/*
  Check on timedRuns, stop runs on expiration time
*/
void checkTimedRun(){
    // check for running command
    if (commandRunning[CR_END_TIME] > 0){
       // a command is running, check for time end
       Serial.println("No command is running");
       if (millis() >= commandRunning[CR_END_TIME]){
         // close valve: stop zone X
         endTimedRun();
       } else {
         // Flash the LED
         if (millis() >= ledFlashTimer){
           if (ledState == LOW){
             ledState = HIGH;
           } else {
             ledState = LOW;
           }
           // Change the LED state
           digitalWrite(ledIndicator, ledState);

           // reset the timer for the next state change
           ledFlashTimer = millis() + ledFlashInterval;
         }
       }
    } else {
      Serial.println("No commands running");
    }
}

void startTimedRun(int zone, unsigned long seconds){
    // deactivate last zone before starting another
    endTimedRun();

    // select pin (shift object id to base zero):
    int pin = zones[zone - 1];
    Serial.print("requesting run for zone#: ");
    Serial.println(zone);
    Serial.print("starting run on pin:");
    Serial.println(pin);

    // turn on selected zone
    digitalWrite(pin, HIGH);

    // turn on the LED indicator light
    digitalWrite(ledIndicator, HIGH);  // set the LED on
    ledFlashTimer = millis() + 3000;   // set the timer
    ledState = HIGH;                   // set the state

    // set commandRunning parameters
    commandRunning[CR_ZONE_ID]    = zone;
    commandRunning[CR_PIN_ID]     = pin; //BUG?: int to unsigned long?
    commandRunning[CR_START_TIME] = millis();
    commandRunning[CR_END_TIME]   = commandRunning[CR_START_TIME] + (seconds * 1000);
}

void endTimedRun(){
  Serial.print("deactivating zone: ");
  Serial.println(commandRunning[CR_PIN_ID]);

  // turn off the pin for the active zone
  digitalWrite(commandRunning[CR_PIN_ID], LOW);
    
  // record the actual end time for reporting
  commandRunning[CR_END_TIME]   = millis();
    
  // turn off the LED indicator
  digitalWrite(ledIndicator, LOW);    // set the LED off
  ledState = LOW;                     // reset the state
  ledFlashTimer = 0;                  // reset the timer
  // clear the commandRunning array
  clearCommandRunning();
}

// shutdown all zones
void shutDownAll(){

  // check if run is in progress first
  if (commandRunning[CR_PIN_ID] != 0){
    // shut down the currently running zone
    endTimedRun();
  }

  // cycle through all pins/zones
  // for good measure
  for (int i = 0; i < zoneCount; i++){
     Serial.print("Writing pin ");
     Serial.print(zones[i]);
     Serial.println(" LOW");

     digitalWrite(zones[i], LOW);
  }
}
