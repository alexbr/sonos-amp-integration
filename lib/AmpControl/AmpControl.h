/**
 * AmpControl v0.1
 *
 * This is an IR/12V trigger controller for an amplifier. This particular code
 * is written for a Yamaha A-S2100. 12V trigger behavior is a real pain in the
 * ass. Also, no discrete power on or off codes.
 *
 * Author: Alex Rodriguez
 */
#ifndef AmpControl_h
#define AmpControl_h

#include <Arduino.h>

/*
#define CMD_PWR_ON 0
#define CMD_PWR_OFF 1
#define CMD_VOL_UP 2
#define CMD_VOL_DOWN 3
#define CMD_MUTE 4
#define CMD_TUNER 5
#define CMD_PHONO 6
#define CMD_MAIN 7
#define CMD_UNKNOWN 255
*/

#define SRC_UNKNOWN 0
#define SRC_TUNER 1
#define SRC_PHONO 2
#define SRC_MAIN 3
#define SRC_BAL 4
#define DEBOUNCE_DELAY_MS 3000
#define IR_POWER_ON_DELAY_MS 800
#define TRIGGER_POWER_ON_DELAY_MS 4000 // to account for the ridiculous amount of time the yamaha takes to switch to main direct
#define IR_SIZE 67

class AmpControl {

   public:
      AmpControl(char irPin);
      AmpControl(char irPin, char triggerPin);

      void turnOn();
      // Turn on avoiding on->off->on transitions
      void turnOnWithAntiFlap();
      void turnOff();
      // Turn off avoiding on->off->on transitions
      void turnOffWithAntiFlap();
      void volumeUp();
      void volumeDown();
      void mute();
      void tuner();
      void phono();
      void bal();
      void mainDirect();
      bool isAmpOn();
      bool isTunerOn();
      bool isBalOn();
      bool isPhonoOn();
      void sendIRCode(const unsigned int *c);

   private:
      char irPin;
      char triggerPin;
      bool useTrigger;
      bool ampOn;
      char source;
      unsigned long offAfterMs;
      unsigned long onAfterMs;

      void init(bool useTrigger);
      void readWords(int *output, const unsigned int *input, const int size);
};

#endif
