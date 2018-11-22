#include <Arduino.h>
#include <MIDI.h>
#include <TimerOne.h>


// Created and binds the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();

// MIDI clock, start and stop byte definitions - based on MIDI 1.0 Standards.
#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP  0xFC

#define CLOCKS_PER_QUARTER_NOTE 24

int CHANNEL = 1;
bool DEBUG = false;

int new_state = 0;
int old_state = 0;

int bpm;

unsigned long startTime;
unsigned long endTime;
unsigned long duration;

byte timerRunning = 0;
int step = 0;

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
  Serial.write(MIDI_START);
}

void loop() {

  

}
