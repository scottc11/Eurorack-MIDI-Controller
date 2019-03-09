#include <Arduino.h>
#include <MIDI.h>
#include <Wire.h>

#define ENCODER_DO_NOT_USE_INTERRUPTS // so Encoder library doesn't use interupts
#include <Encoder.h>


// Creates and binds the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();

// MIDI clock, start and stop byte definitions - based on MIDI 1.0 Standards.
#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP  0xFC


Encoder encoder1(9, 12);


int MIDI_START_PIN = 4;
int MIDI_STOP_PIN = 5;

int MIDI_CHANNEL = 1;
bool DEBUG = false;


bool started = false;

// Trigger Input Pins
int triggerInputCount = 3;
int trigger_input_pins[] = {6, 7, 8, 5};  // *** MCP23017 pins

int buttonMIDIValues[] = {21, 37, 53, 77};


int counter[] = {0, 0, 0, 0}; // for encoder ?

// button states
byte newInputStates = 0;
byte oldInputStates = 0;

long oldPosition1 = -999;


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
    // MIDI.sendTimeCodeQuarterFrame(DataByte inTypeNibble, DataByte inValuesNibble);
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

  // INIT TRIGGER INPUTS
  for (uint8_t i = 0; i < triggerInputCount; i++) {
    pinMode(trigger_input_pins[i], INPUT);
  }
}

void loop() {

  long newPosition1 = encoder1.read();

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




  // get state of each input
  for (uint8_t i=0; i < triggerInputCount; i++) {
    uint8_t state = digitalRead(trigger_input_pins[i]);
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
