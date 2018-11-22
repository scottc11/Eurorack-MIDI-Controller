
### getting the interval between pin inputs.

```
new_state = digitalRead(4);
// one step == one 16th note.  4 steps = 1 quarter note.
if (new_state != old_state) {

  // when input goes LOW, start the timer
  if (new_state == LOW && timerRunning == 0) {
    startTime = millis();
    timerRunning = 1;
  }

  // when input goes HIGH, stop the timer
  if (new_state == HIGH && timerRunning == 1) {
    endTime = millis();
    timerRunning = 0;
    duration = endTime - startTime;

    step += 1;

    if (DEBUG) {
      Serial.println(duration);Serial.print("   ::::    ");
      Serial.print(startTime);Serial.print("   ::::    ");
      Serial.print(endTime);Serial.print("   ::::    ");
    }

  }
  old_state = new_state;
}
```
