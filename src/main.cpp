#include <Arduino.h>
#include <MIDI.h>
#include <Wire.h>
#include <Adafruit_MCP23017.h>

#define ENCODER_DO_NOT_USE_INTERRUPTS // so Encoder library doesn't use interupts
#include <Encoder.h>


// Creates and binds the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();

// MIDI clock, start and stop byte definitions - based on MIDI 1.0 Standards.
#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP  0xFC

#define IO_ADDR 0x00

Adafruit_MCP23017 io = Adafruit_MCP23017();

Encoder encoder(9, 12);


int MIDI_START_PIN = 4;
int MIDI_STOP_PIN = 5;

int MIDI_CHANNEL = 1;
bool DEBUG = false;


bool started = false;

// Trigger Input Pins
int triggerInputCount = 3;
int trigger_input_pins[] = {4, 5, 6, 7};
int channel_leds[] = {0, 1, 2, 3};

int triggerMIDIValues[] = {21, 37, 53, 77};


int counter[] = {0, 0, 0, 0}; // for encoder ?

// button states
byte newInputStates = 0;
byte oldInputStates = 0;

long oldEncoderPos = -999;
int newEndcoderPushState = 0;
int oldEndcoderPushState = 0;


int selected_trigger = 0;        // what trigger the encoder will be acting on.
int NUM_TRIG_INPUTS = 4;         // how many trigger inputs the software will accept


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

  io.begin(IO_ADDR);

  // init IO LEDS
  for (uint8_t i = 0; i < NUM_TRIG_INPUTS; i++) {
    io.pinMode(channel_leds[i], OUTPUT);

    if (i == selected_trigger) {
      io.digitalWrite(channel_leds[i], HIGH);
    } else {
      io.digitalWrite(channel_leds[i], LOW);
    }
  }

  // INIT TRIGGER INPUTS
  for (uint8_t i = 0; i < triggerInputCount; i++) {
    pinMode(trigger_input_pins[i], INPUT);
  }
}





void loop() {

  long newEncoderPos = encoder.read();

  newEndcoderPushState = digitalRead(10);

  if (newEndcoderPushState != oldEndcoderPushState) {

    // if the encoder button is currently pressed down
    if (newEndcoderPushState == HIGH) {
      if (selected_trigger == 3) {
        selected_trigger = 0;
      } else {
        selected_trigger += 1;
      }

      for (uint8_t i = 0; i < NUM_TRIG_INPUTS; i++) {
        if (i == selected_trigger) {
          io.digitalWrite(i, HIGH);
        } else {
          io.digitalWrite(i, LOW);
        }
      }
      if (DEBUG) { Serial.println(selected_trigger); }
    }

    // if the encoder button is released
    else {
      // do nothing
    }

    oldEndcoderPushState = newEndcoderPushState;
  }

  // ENCODER ONE
  if (newEncoderPos != oldEncoderPos ) {
    if (DEBUG) { Serial.println(newEncoderPos); }

    if (newEncoderPos % 4 == 0) {
      if (oldEncoderPos > newEncoderPos) {
        counter[selected_trigger] += 1;
        triggerMIDIValues[selected_trigger] += 1;
      } else {
        counter[selected_trigger] -= 1;
        triggerMIDIValues[selected_trigger] -= 1;
      }
    }
    oldEncoderPos = newEncoderPos;
  }




  // get state of each input
  for (uint8_t i=0; i < triggerInputCount; i++) {
    uint8_t state = digitalRead(trigger_input_pins[i]);
    bitWrite(newInputStates, i, state);
  }

  if ( newInputStates != oldInputStates ) {

    for (uint8_t i=0; i < triggerInputCount; i++) {

      if (bitRead(newInputStates, i) != bitRead(oldInputStates, i)) {
        if (bitRead(newInputStates, i) == LOW) { // LOW means a trigger / gate has been detected
          if (DEBUG) {
            Serial.print("channel: "); Serial.print(i);
            Serial.print(" -- MIDI note: "); Serial.print(triggerMIDIValues[i]);
            Serial.println(" ");
          }
          MIDI.sendNoteOn(triggerMIDIValues[i], 127, MIDI_CHANNEL);
        } else {
          if (DEBUG) { Serial.println("LOW"); }
          MIDI.sendNoteOff(triggerMIDIValues[i], 0, MIDI_CHANNEL);
        }
      }
    }

    oldInputStates = newInputStates;
  }

}
