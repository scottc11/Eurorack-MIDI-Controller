#include <Arduino.h>
#include <MIDI.h>
#include <TimerOne.h>

#define ENCODER_DO_NOT_USE_INTERRUPTS // so Encoder library doesn't use interupts
#include <Encoder.h>
#include <Adafruit_MCP23017.h>


// Creates and binds the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();
Adafruit_MCP23017 mcp = Adafruit_MCP23017();

// MIDI clock, start and stop byte definitions - based on MIDI 1.0 Standards.
#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP  0xFC


Encoder encoder1(9, 12);
Encoder encoder2(8, 11);
Encoder encoder3(7, 10);
// Encoder encoder4(6, 13);
int MIDI_START_PIN = 4;
int MIDI_STOP_PIN = 5;

int MIDI_CHANNEL = 1;
bool DEBUG = false;


bool started = false;

// Trigger Input Pins
int triggerInputCount = 4;
int trigger_input_pins[] = {8, 9, 10, 11};  // *** MCP23017 pins

int buttonMIDIValues[] = {21, 37, 53, 77};


int counter[] = {0, 0, 0, 0}; // for encoder ?

// button states
byte newInputStates = 0;
byte oldInputStates = 0;

long oldPosition1 = -999;
long oldPosition2 = -999;
long oldPosition3 = -999;
// long oldPosition4 = -999;

int step = 0;

int startNewState = 0;
int startOldState = 0;

int stopNewState = 0;
int stopOldState = 0;

// Start the slave MIDI device
void startClock() {
  if (DEBUG) { Serial.println("------ START CLOCK -------"); }
  Serial.write(MIDI_START);
  started = true;
}

// Stop the slave MIDI device
void stopClock() {
  if (DEBUG) { Serial.println("------ STOP CLOCK -------"); }
  Serial.write(MIDI_STOP);
  started = false;
}


// Interupt Callback Function
void sendClockPulse() {
  step += 1;
  if (step % 4 == 1) {
    // for (size_t i = 0; i < 24; i++) {
    //   Serial.write(MIDI_CLOCK);
    // }
    MIDI.sendTimeCodeQuarterFrame(DataByte inTypeNibble, DataByte inValuesNibble)
  }
}



void setup() {
  // put your setup code here, to run once:
  if (!DEBUG) {
    Serial.begin(31250);
    MIDI.begin(MIDI_CHANNEL_OMNI);  // Listen to all incoming messages
  } else {
    Serial.begin(9600);
  }

  // DIGITAL PIN 3 is used for detecting tempo.
  attachInterrupt(digitalPinToInterrupt(3), sendClockPulse, RISING);


  pinMode(MIDI_START_PIN, INPUT); // MIDI START button
  pinMode(MIDI_STOP_PIN, INPUT); // MIDI STOP button

  // INIT TRIGGER INPUTS
  mcp.begin();
  for (size_t i = 0; i < triggerInputCount; i++) {
    mcp.pinMode(trigger_input_pins[i], INPUT);
  }
}

void loop() {

  if (digitalRead(MIDI_START_PIN) == HIGH && !started) {
    startClock();
  }
  if (digitalRead(MIDI_STOP_PIN) == HIGH && started) {
    stopClock();
  }


  long newPosition1 = encoder1.read();
  long newPosition2 = encoder2.read();
  long newPosition3 = encoder3.read();
  // long newPosition4 = encoder4.read();

  // ENCODER ONE
  if (newPosition1 != oldPosition1 ) {
    if (DEBUG) { Serial.println(newPosition1); }

    if (newPosition1 % 4 == 0) {
      if (oldPosition1 > newPosition1) {
        counter[0] += 1;
      } else {
        counter[0] -= 1;
      }
    }
    oldPosition1 = newPosition1;
  }

  // ENCODER TWO
  if (newPosition2 != oldPosition2 ) {
    if (DEBUG) { Serial.println(newPosition2); }

    if (newPosition2 % 4 == 0) {
      if (oldPosition2 > newPosition2) {
        counter[1] += 1;
      } else {
        counter[1] -= 1;
      }
    }
    oldPosition2 = newPosition2;
  }

  // ENCODER THREE
  if (newPosition3 != oldPosition3 ) {
    if (DEBUG) { Serial.println(newPosition3); }

    if (newPosition3 % 4 == 0) {
      if (oldPosition3 > newPosition3) {
        counter[2] += 1;
      } else {
        counter[2] -= 1;
      }
    }
    oldPosition3 = newPosition3;
  }

  // ENCODER FOUR
  // if (newPosition4 != oldPosition4 ) {
  //   if (DEBUG) { Serial.println(newPosition4); }
  //
  //   if (newPosition4 % 4 == 0) {
  //     if (oldPosition4 > newPosition4) {
  //       counter[3] += 1;
  //     } else {
  //       counter[3] -= 1;
  //     }
  //   }
  //   oldPosition4 = newPosition4;
  // }




  // get state of each input
  for (uint8_t i=0; i < triggerInputCount; i++) {
    uint8_t state = mcp.digitalRead(trigger_input_pins[i]);
    bitWrite(newInputStates, i, state);
  }

  if ( newInputStates != oldInputStates ) {

    for (uint8_t i=0; i < triggerInputCount; i++) {

      if (bitRead(newInputStates, i) != bitRead(oldInputStates, i)) {
        if (bitRead(newInputStates, i) == LOW) {
          if (DEBUG) { Serial.println("HIGH"); }
          MIDI.sendNoteOn(buttonMIDIValues[i] + counter[i], 127, MIDI_CHANNEL);
        } else {
          if (DEBUG) { Serial.println("HIGH"); }
          MIDI.sendNoteOff(buttonMIDIValues[i] + counter[i], 0, MIDI_CHANNEL);
        }
      }
    }

    oldInputStates = newInputStates;
  }

}
