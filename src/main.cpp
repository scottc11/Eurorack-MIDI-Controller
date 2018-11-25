#include <Arduino.h>
#include <MIDI.h>
#include <TimerOne.h>
#include <Encoder.h>
#include <Adafruit_MCP23017.h>


// Created and binds the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();
Adafruit_MCP23017 mcp = Adafruit_MCP23017();

// MIDI clock, start and stop byte definitions - based on MIDI 1.0 Standards.
#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP  0xFC

Encoder encoder1(9, 12);
Encoder encoder2(8, 11);
Encoder encoder3(7, 10);

int MIDI_CHANNEL = 1;
bool DEBUG = true;


bool started = false;

// Trigger Input Pins
int inputCount = 3;
int inputPins[] = {2, 4, 5};
int buttonMIDIValues[] = {21, 37, 53};
int counter[] = {0, 0, 0}; // for encoder ?

// button states
byte newButtonStates = 0;
byte oldButtonStates = 0;

long oldPosition1 = -999;
long oldPosition2 = -999;
long oldPosition3 = -999;

int step = 0;

int startNewState = 0;
int startOldState = 0;

int stopNewState = 0;
int stopOldState = 0;

// Start the slave MIDI device
void startClock() {
  Serial.write(MIDI_START);
  started = true;
}

// Stop the slave MIDI device
void stopClock() {
  Serial.write(MIDI_STOP);
  started = false;
}


// Interupt Callback Function
void sendClockPulse() {
  step += 1;
  if (step % 4 == 1) {
    for (size_t i = 0; i < 24; i++) {
      Serial.write(MIDI_CLOCK);
    }
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

  attachInterrupt(digitalPinToInterrupt(3), sendClockPulse, FALLING);

  // mcp.begin();

  // Connect MCP23017 pinouts
  // mcp.pinMode(8, INPUT);

  // BUTTONS
  pinMode(2, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
}

void loop() {

  if (digitalRead(6) == HIGH && !started) {
    startClock();
  }

  long newPosition1 = encoder1.read();
  long newPosition2 = encoder2.read();
  long newPosition3 = encoder3.read();

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




  // get switch states of each scale step
  for (uint8_t i=0; i < inputCount; i++) {
    uint8_t state = digitalRead(inputPins[i]);
    bitWrite(newButtonStates, i, state);
  }

  if ( newButtonStates != oldButtonStates ) {

    for (uint8_t i=0; i < inputCount; i++) {

      if (bitRead(newButtonStates, i) != bitRead(oldButtonStates, i)) {
        if (bitRead(newButtonStates, i) == LOW) {
          if (DEBUG) { Serial.println("HIGH"); }
          MIDI.sendNoteOn(buttonMIDIValues[i] + counter[i], 127, MIDI_CHANNEL);
        } else {
          if (DEBUG) { Serial.println("HIGH"); }
          MIDI.sendNoteOff(buttonMIDIValues[i] + counter[i], 0, MIDI_CHANNEL);
        }
      }
    }

    oldButtonStates = newButtonStates;
  }

}
