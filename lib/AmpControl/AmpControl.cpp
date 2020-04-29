/**
 * AmpControl v0.1
 *
 * This is an IR/12V trigger controller for an amplifier. This particular code
 * is written for a Yamaha A-S2100. 12V trigger behavior is a real pain in the
 * ass. Also, no discrete power on or off codes.
 *
 * Author: Alex Rodriguez
 */
#include "AmpControl.h"
#include <Wire.h>

const unsigned int pwrCode[IR_SIZE] PROGMEM = { 9000, 4500, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560 };
const unsigned int muteCode[IR_SIZE] PROGMEM = { 9000, 4500, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 560, 560 };
const unsigned int volUpCode[IR_SIZE] PROGMEM = { 9000, 4500, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560 };
const unsigned int volDownCode[IR_SIZE] PROGMEM = { 9000, 4500, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560 };
const unsigned int tunerCode[IR_SIZE] PROGMEM = { 9000, 4500, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560 };
const unsigned int phonoCode[IR_SIZE] PROGMEM = { 9000, 4500, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560 };
const unsigned int mainCode[IR_SIZE] PROGMEM = { 9000, 4500, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 560, 560 };

/**
 * Constructor that takes the IR pin. This will be used for all communication
 * to the amplifier.
 */
AmpControl::AmpControl(char irPin) {
   this->irPin = irPin;
   this->init(false);
}

/**
 * Constructor that also takes a 12V trigger pin. This is preferred over IR
 * power on/off toggle since it is discrete; the amp will definitively be on
 * or off depending on the trigger state.
 */
AmpControl::AmpControl(char irPin, char triggerPin) {
   this->irPin = irPin;
   this->triggerPin = triggerPin;
   this->init(true);
}

void AmpControl::init(bool useTrigger) {
   this->useTrigger = useTrigger;
   this->ampOn = false;
   this->source = SRC_UNKNOWN;
   this->offAfterMs = 0; // 0 is "not set"
   this->onAfterMs = 0; // 0 is "not set"
}

bool AmpControl::isAmpOn() {
   return this->ampOn;
}

bool AmpControl::isTunerOn() {
   return this->source == SRC_TUNER;
}

bool AmpControl::isPhonoOn() {
   return this->source == SRC_PHONO;
}

void AmpControl::turnOn() {
   if (this->useTrigger) {
      digitalWrite(this->triggerPin, HIGH);
      // because yamaha...
      this->source = SRC_MAIN;
   } else if (!this->ampOn) {
      this->sendIRCode(pwrCode);
   }

   this->ampOn = true;
   this->onAfterMs = 0;
   this->offAfterMs = 0;
}

// TODO: this anti-flap "detection" is flaky and confusing and doesn't belong
// here. Replace it with rate limiting in the client.
void AmpControl::turnOnWithAntiFlap() {
   // onAfterMs is set by turnOff to prevent flapping back on after turning off
   if (millis() < this->onAfterMs) {
      return;
   }

   this->turnOn();
}

void AmpControl::turnOff() {
   if (this->useTrigger) {
      // This is required since the 12V trigger is ignored after source is
      // switched away from main direct.
      this->mainDirect();
      delay(200); // wait for source to switch
      digitalWrite(this->triggerPin, LOW);
      // Give the amp time to shut down - if trigger on comes in before this
      // delay, the amp will ignore it putting it in a wierd state where 12V
      // trigger is on but the amp is off. This plus the whole main direct
      // source requirement is fucking obnoxious.
      // Not really necessary if debounce delay is sufficiently long.
      // delay(700);
   } else if (this->ampOn) {
      this->sendIRCode(pwrCode);
   }

   this->ampOn = false;
   this->offAfterMs = 0;
   // Prevent flapping back on
   this->onAfterMs = millis() + DEBOUNCE_DELAY_MS;
}

// Debounce amp off requests to avoid flapping
void AmpControl::turnOffWithAntiFlap() {
   if (this->offAfterMs == 0) {
      this->offAfterMs = millis() + DEBOUNCE_DELAY_MS;
      return;
   }

   if (millis() < this->offAfterMs) {
      return;
   }

   this->turnOff();
}

void AmpControl::mute() {
   this->sendIRCode(muteCode);
}

void AmpControl::volumeUp() {
   this->sendIRCode(volUpCode);
}

void AmpControl::volumeDown() {
   this->sendIRCode(volDownCode);
}

void AmpControl::mainDirect() {
   this->sendIRCode(mainCode);
   this->source = SRC_MAIN;
}

void AmpControl::tuner() {
   this->sendIRCode(tunerCode);
   this->source = SRC_TUNER;
}

void AmpControl::phono() {
   this->sendIRCode(phonoCode);
   this->source = SRC_PHONO;
}

void AmpControl::sendIRCode(const unsigned int *c) {
   int code[IR_SIZE] = {};
   this->readWords(code, c, IR_SIZE);

   for (int i = 0; i < IR_SIZE; i++) {
      if (i % 2 == 0) { // Set mark
         digitalWrite(this->irPin, HIGH);
         delayMicroseconds(code[i]);
      }
      else { // set space
         digitalWrite(this->irPin, LOW);
         delayMicroseconds(code[i]);
      }
   }

   digitalWrite(this->irPin, LOW);
}

void AmpControl::readWords(int *output, const unsigned int *input, const int size) {
   for (int i = 0; i < size; i++) {
      output[i] = pgm_read_word_near(input + i);
   }
}
