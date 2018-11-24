#include <Arduino.h>
#include <MIDI.h>
#include <TimerOne.h>
#include <Encoder.h>


// Created and binds the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();

// MIDI clock, start and stop byte definitions - based on MIDI 1.0 Standards.
#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP  0xFC

Encoder encoder1(9, 12);
Encoder encoder2(8, 11);
Encoder encoder3(7, 10);

int CHANNEL = 1;
bool DEBUG = false;
bool started = false;

int inputCount = 4;
int inputPins[] = {2, 4, 5, 6};
int buttonMIDIValues[] = {21, 37, 53, 69};
int counter[] = {0, 0, 0, 0};

// button states
byte newButtonStates = 0;
byte oldButtonStates = 0;

long oldPosition1 = -999;
long oldPosition2 = -999;
long oldPosition3 = -999;

int step = 0;


// Start the slave device
void startClock() {
  Serial.write(MIDI_START);
  started = true;
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


  // BUTTONS
  pinMode(2, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
}

void loop() {

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
          MIDI.sendNoteOn(buttonMIDIValues[i] + counter[i], 127, CHANNEL);
        } else {
          if (DEBUG) { Serial.println("HIGH"); }
          MIDI.sendNoteOff(buttonMIDIValues[i] + counter[i], 0, CHANNEL);
        }
      }
    }

    oldButtonStates = newButtonStates;
  }

}
