/********************************************************

  Alex's sonos and amp controller

 *********************************************************/

#include "main.h"
#include "secrets.h"
#include <Adafruit_RGBLCDShield.h>
#include <AmpControl.h>
#if WIFI
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiClient.h>
#else
#include <Internet.h>
#endif
#include <LCDHelper.h>
#include <MicroXPath_P.h>
#include <Sonos.h>
#if TIME
#include <WiFiUDP.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <Timezone.h>
#endif
#include <Wire.h>
#include <avr/wdt.h>
#include <utility/Adafruit_MCP23017.h>

// Internet
const char connError[] PROGMEM = "connect error";
const char ipFormat[] PROGMEM = "%d.%d.%d.%d";
const char connectAttempt[] PROGMEM = "attempting to connect...";
const char connectReason[] PROGMEM = "reason code: ";

// HTTP Server
const char httpGet[] PROGMEM = "GET /";
const char muteUri[] PROGMEM = "mute";
const char volupUri[] PROGMEM = "volup";
const char voldownUri[] PROGMEM = "voldown";
const char tunerUri[] PROGMEM = "tuner";
const char balUri[] PROGMEM = "bal";
const char phonoUri[] PROGMEM = "phono";
const char onnUri[] PROGMEM = "pwron";
const char offUri[] PROGMEM = "pwroff";
const char httpRequest[] PROGMEM =
    "HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\n\nGot URI ";

// LCD
const char phonoOverride1[] PROGMEM = "Record player";
const char phonoOn2[] PROGMEM = "on";
const char phonoOff2[] PROGMEM = "off";
const char playing[] PROGMEM = "Playing";
const char paused[] PROGMEM = "Paused";
const char stopped[] PROGMEM = "Stopped";
const char unknown[] PROGMEM = "Unknown";
const char powerOn[] PROGMEM = "Power on";
const char powerOff[] PROGMEM = "Power off";
const char volumeUp[] PROGMEM = "Volume up";
const char volumeDown[] PROGMEM = "Volume down";
const char mute[] PROGMEM = "Mute";
const char tuner[] PROGMEM = "Tuner";
const char phono[] PROGMEM = "Phono";
const char by[] PROGMEM = " by ";

const char play[] PROGMEM = "Play";
const char pause[] PROGMEM = "Pause";
const char previous[] PROGMEM = "Previous";
const char next[] PROGMEM = "Next";
const char ip[] PROGMEM = "IP";
const char connectingInternet[] PROGMEM = "connecting internet...";
const char internetInitialized[] PROGMEM = "internet initialized";
const char connecting[] PROGMEM = "Connecting...";
const char noWiFiModule[] PROGMEM = "No WiFi module";

const char empty[] PROGMEM = "";
const char rebooting[] PROGMEM = "rebooting...";

// Internet setup
IPAddress gatewayIP(192, 168, 10, 1);
#if WIFI
unsigned long pingAfter = 0;
WiFiServer server(80);
WiFiClient sonosClient;
#else
const int* MAC[] PROGMEM = {0xA8, 0x61, 0x0A, 0xAE, 0x5D, 0x54};
InternetClient sonosClient;
InternetServer server(80);
#endif

// Amp control
AmpControl amp = AmpControl(IR_PIN_OUT, TRIGGER_PIN_OUT);
char intendedSource = SRC_UNKNOWN;
unsigned long checkSourceAfter = 0;

// Sonos setup
IPAddress sonosIP;
Sonos sonos = Sonos(sonosClient, connectError);

// LCD
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
LCDHelper lcdHelper = LCDHelper(lcd);

#if TIME
// time.nist.gov NTP server
IPAddress timeServer(129, 6, 15, 28);
WiFiUDP ntpUdp;
NTPClient ntpClient(ntpUdp, timeServer);
unsigned long checkTimeAfter = 0;
TimeChangeRule myPST = {"PST", Second, Sun, Mar, 2, -7 * 60}; // Daylight time = UTC - 7 hours
TimeChangeRule myPDT = {"PDT", First, Sun, Nov, 2, -8 * 60};  // Standard time = UTC - 8 hours
Timezone myTZ(myPST, myPDT);
bool screenOff = false;
#endif

// Declare reboot function at address 0
void(*reboot) (void) = 0;

void setup() {
   Serial.begin(9600);

   pinMode(IR_PIN_OUT, OUTPUT);
   pinMode(TRIGGER_PIN_OUT, OUTPUT);
   
   // Set up the LCD's columns and rows
   lcd.begin(16, 2);

   printStringLnP(connectingInternet);
   connect();

   getSonosIP(sonosIP, livingRoomSonos);

   server.begin();
   
   printStringLnP(internetInitialized);
#if WIFI
   Serial.println(WiFi.localIP());
#else
   Serial.println(Internet.localIP());
#endif

#if TIME
   ntpClient.begin();
   setSyncProvider(getTime);
#endif
}

void loop() {
   // Try to connect if connection appears to be down; reboot if no bueno. 
   if (!checkConnectionStatus(false) && !connect()) {
      printStringLnP(rebooting);
      reboot();
   }

   // Check inputs in order of precedence, skipping additional checks if any
   // prior registered input.
   bool gotInput = checkButtons();

   if (!gotInput) {
      gotInput = checkServer();
   }

   if (!gotInput) {
      checkSource();
   }


#if TIME
   if (millis() > checkTimeAfter) {
      checkTimeAfter = millis() + CHECK_TIME_DELAY_MS;
      
      time_t t = myTZ.toLocal(now());
      int hhmm = hour(t) * 100 + minute(t);
      Serial.println(hhmm);
      if (hhmm >= LCD_OFF_AFTER || hhmm < LCD_ON_AFTER) {
         screenOff = true;
      } else {
         screenOff = false;
      }
   }
#endif

   if (screenOff) {
      lcdHelper.screenOff();
   } else {
      lcdHelper.screenOn();
      lcdHelper.print();
   }
}

bool checkConnectionStatus(bool forcePing) {
#if WIFI
   if (millis() > pingAfter) {
      forcePing = true;
      pingAfter = millis() + PING_DELAY_MS;
   }
   return WiFi.status() == WL_CONNECTED && (!forcePing || WiFi.ping(gatewayIP) >= 0);
#else 
   return Internet.connected();
#endif
}

bool connect() {
   printStringLnP(connecting);
   lcdHelper.printNextP(connecting, empty, RED, 0);
   lcdHelper.print();
   
#if WIFI
   if (WiFi.status() == WL_NO_MODULE) {
      printStringLnP(noWiFiModule);
      return false;
   }

   char *ssid = SECRET_SSID;
   char *passkey = SECRET_PASSKEY;
   String fv = WiFi.firmwareVersion();
   String latestFv = WIFI_FIRMWARE_LATEST_VERSION;

   Serial.println(fv);
   Serial.println(WIFI_FIRMWARE_LATEST_VERSION);

   int status = WiFi.disconnect();
   //WiFi.end();

   // Give this guy a long time to connect, auth seems to be
   // slow with my AP
   uint8_t tries = 0;
   bool connected = false;

   while (!connected && tries < WIFI_CONNECT_TRIES) {
      status = WiFi.disconnect();
      printStringLnP(connectAttempt);
      status = WiFi.begin(ssid, passkey);
      printStringP(connectReason);
      Serial.println(WiFi.reasonCode());
      printString("tries: ");
      Serial.println(tries);

      unsigned long tryUntil = millis() + WIFI_CONNECT_TIMEOUT_MS;
      while (!(connected = checkConnectionStatus(true)) && millis() < tryUntil) {
         delay(250);
      }

      tries++;
   }

   if (connected) {
      Serial.println("connect ok");
      WiFi.noLowPowerMode();
      return true;
   } else {
      Serial.println("connection failed");
   }

   return false;
#else
   int* mac[6] = {};
   readBytes(mac, MAC, 6);

   if (!Internet.begin(mac)) {
      const IPAddress staticIP(192, 168, 10, 245);
      // printStringLn("starting with MAC failed...");
      Internet.begin(mac, staticIP);
      return true;
   }
#endif
}

bool checkButtons() {
   bool gotButton = false;
   char buttons = lcd.readButtons();

   if (buttons) {
      gotButton = true;
      char row1[10];
      unsigned long displayUntil = millis() + BUTTON_PRESS_VIEW_DURATION_MS;

      if (buttons & BUTTON_UP) {
         strcpy_P(row1, play);
         lcdHelper.printNext(row1, "", VIOLET, displayUntil);
         sonos.play(sonosIP);
      }
      if (buttons & BUTTON_DOWN) {
         strcpy_P(row1, pause);
         lcdHelper.printNext(row1, "", RED, displayUntil);
         sonos.pause(sonosIP);
      }
      if (buttons & BUTTON_LEFT) {
         strcpy_P(row1, previous);
         lcdHelper.printNext(row1, "", YELLOW, displayUntil);
         sonos.skip(sonosIP, 0); // back
      }
      if (buttons & BUTTON_RIGHT) {
         strcpy_P(row1, next);
         lcdHelper.printNext(row1, "", TEAL, displayUntil);
         sonos.skip(sonosIP, 1); // forward
      }
      if (buttons & BUTTON_SELECT) {
         if (intendedSource == SRC_PHONO) {
            phonoOff();
            amp.turnOffWithAntiFlap();
         } else {
            amp.turnOn();
            phonoOn();
         }
      }

      // Debounce button press (doc claims unnecessary, but testing proves
      // otherwise)
      delay(200);
   }

   return gotButton;
}

bool checkServer() {
   bool gotCmd = false;
#if WIFI
   WiFiClient client = server.available();
#else
   InternetClient client = server.available();
#endif

   if (client) {
      char get[strlen_P(httpGet) + 1];
      strcpy_P(get, httpGet);
      char uri[16] = "";
      unsigned int index = 0; // used for method and URI indexing

      boolean collectMethod = true;
      boolean collectUri = true;
      boolean currentLineIsBlank = false;

      while (client.connected() && client.available()) {
         char c = client.read();
         if (c == '\n' && currentLineIsBlank) {
            char req[strlen_P(httpRequest) + 1];
            strcpy_P(req, httpRequest);
            client.print(req);
            client.println(uri);
            break;
         }

         if (c == '\n') {
            currentLineIsBlank = true;
         } else if (c != '\r') {
            currentLineIsBlank = false;

            if (collectMethod) {
               if (c == get[index]) {
                  index++;
                  if (index == strlen(get)) {
                     collectMethod = false;
                     index = 0;
                  }
               }
            } else if (collectUri && index < 15) {
               if (c == ' ') {
                  uri[index] = '\0';
                  collectUri = false;
               } else {
                  uri[index] = c;
                  index++;
               }
            }
         }
      }

      if (strlen(uri) > 0) {
         char row[LCD_ROW_STR_LENGTH];
         unsigned long displayUntil = millis() + BUTTON_PRESS_VIEW_DURATION_MS;

         if (strcmp_P(uri, muteUri) == 0) {
            lcdHelper.printNext(strcpy_P(row, mute), "", GREEN, displayUntil);
            amp.mute();
         } else if (strcmp_P(uri, volupUri) >= 0) {
            lcdHelper.printNext(strcpy_P(row, volumeUp), "", GREEN,
                                displayUntil);
            const uint8_t volSteps = getStepsFromUri(uri);
            for (uint8_t i = 0; i < volSteps; i++) {
               amp.volumeUp();
               delay(20);
            }
         } else if (strcmp_P(uri, voldownUri) >= 0) {
            lcdHelper.printNext(strcpy_P(row, volumeDown), "", GREEN,
                                displayUntil);
            const uint8_t volSteps = getStepsFromUri(uri);
            for (uint8_t i = 0; i < volSteps; i++) {
               amp.volumeDown();
               delay(20);
            }
         } else if (strcmp_P(uri, balUri) == 0) {
            lcdHelper.printNext(strcpy_P(row, powerOn), "", GREEN,
                                displayUntil);
            amp.turnOn();
            balOn();
         } else if (strcmp_P(uri, tunerUri) == 0) {
            lcdHelper.printNext(strcpy_P(row, powerOn), "", GREEN,
                                displayUntil);
            amp.turnOn();
            tunerOn();
         } else if (strcmp_P(uri, phonoUri) == 0) {
            lcdHelper.printNext(strcpy_P(row, phono), "", GREEN, displayUntil);
            amp.turnOn();
            phonoOn();
         } else if (strcmp_P(uri, onnUri) == 0) {
            lcdHelper.printNext(strcpy_P(row, powerOn), "", GREEN,
                                displayUntil);
            amp.turnOn();
         } else if (strcmp_P(uri, offUri) == 0) {
            lcdHelper.printNext(strcpy_P(row, powerOff), "", GREEN,
                                displayUntil);
            intendedSource = SRC_UNKNOWN;
            amp.turnOff();
         }

         gotCmd = true;
      }
   }

   delay(1);
   client.stop();

   return gotCmd;
}

void checkSource() {
   if (millis() < checkSourceAfter) {
      return;
   }

   checkSourceAfter = millis() + SOURCE_STATUS_POLL_DELAY_MS;

   if (intendedSource == SRC_PHONO) {
      amp.turnOn();
      amp.phono();
      lcdHelper.maybePrintNextP(phonoOverride1, phonoOn2, BLUE);
   } else {
      // Sonos state polling
      byte playerState = sonos.getState(sonosIP);
      char uri[20] = "";
      char title[75] = "";
      char artist[40] = "";

      TrackInfo track =
          sonos.getTrackInfo(sonosIP, uri, sizeof(uri), title, sizeof(title),
                             artist, sizeof(artist));

      byte source = sonos.getSourceFromURI(track.uri);
      char sonosRow1[LCD_ROW1_LENGTH];
      char sonosRow2[LCD_ROW2_LENGTH];
      int sonosColor;

      switch (playerState) {
      case SONOS_STATE_PLAYING:
         strcpy_P(sonosRow1, playing);
         sonosColor = VIOLET;
         amp.turnOnWithAntiFlap();
         sonosInput();
         break;
      case SONOS_STATE_PAUSED:
         strcpy_P(sonosRow1, paused);
         sonosColor = YELLOW;
         amp.turnOffWithAntiFlap();
         break;
      case SONOS_STATE_STOPPED:
         strcpy_P(sonosRow1, stopped);
         sonosColor = WHITE;
         amp.turnOffWithAntiFlap();
         break;
      default:
         printStringLn("sonos state unknown");
         strcpy_P(sonosRow1, unknown);
         sonosColor = TEAL;
         break;
      }

      // If the sonos device's source is 'master', switch to the presumed master
      // device.
      if (source == SONOS_SOURCE_MASTER) {
         IPAddress ip;
         getSonosIP(ip, livingRoomSonos);
         if (sonosIP == ip) {
            getSonosIP(sonosIP, kitchenSonos);
         } else {
            getSonosIP(sonosIP, livingRoomSonos);
         }
      }

      if (source != SONOS_SOURCE_LINEIN && playerState != SONOS_STATE_STOPPED) {
         strcpy(sonosRow2, title);
         if (strlen(artist) > 0 && LCD_ROW2_LENGTH - strlen(sonosRow2) >= 10) {
            strcat_P(sonosRow2, by);
            strncat(sonosRow2, artist,
                    min(LCD_ROW2_LENGTH - strlen(sonosRow2), strlen(artist)));
         }
      } else {
         sonosRow2[0] = '\0';
      }

      lcdHelper.maybePrintNext(sonosRow1, sonosRow2, sonosColor);
   }
}

uint8_t getStepsFromUri(char *uri) {
   char steps[4] = "";
   boolean foundSteps = false;

   for (uint8_t i = 0; i < strlen(uri) && strlen(steps) < 4; i++) {
      char c = uri[i];
      if (foundSteps == true && isDigit(c)) {
         steps[strlen(steps)] = c;
         steps[strlen(steps) + 1] = '\0';
      } else if (c == '/') {
         foundSteps = true;
      }
   }

   uint8_t volSteps = 5;
   if (strlen(steps) > 0) {
      volSteps = atoi(steps);
   }

   return volSteps;
}

void sonosInput() {
   if (strcmp_P(SONOS_INPUT, balUri) == 0) {
      balOn();
   } else if (strcmp_P(SONOS_INPUT, tunerUri) == 0) {
      tunerOn();
   }
}

void balOn() {
   intendedSource = SRC_BAL;
   amp.bal();
}

void tunerOn() {
   intendedSource = SRC_TUNER;
   amp.tuner();
}

void phonoOn() {
   intendedSource = SRC_PHONO;
   amp.phono();
   lcdHelper.printNextP(phonoOverride1, phonoOn2, BLUE, 0);
}

void phonoOff() {
   intendedSource = SRC_UNKNOWN;
   lcdHelper.printNextP(phonoOverride1, phonoOff2, TEAL, 0);
}

#if TIME
unsigned long getTime() {
   return ntpClient.getTime();
}
#endif

void readBytes(byte *output, const byte *input, const int size) {
   for (int i = 0; i < size; i++) {
      output[i] = pgm_read_byte_near(input + i);
   }
}

void readWords(int *output, const int *input, const int size) {
   for (int i = 0; i < size; i++) {
      output[i] = pgm_read_word_near(input + i);
   }
}

void printString(const char *str) {
   const char *p = str;
   while (*p) {
      Serial.print(*p);
      p++;
   }
}

void printStringLn(const char *str) {
   printString(str);
   Serial.print('\n');
}

void printStringP(const char *str) {
   char c[strlen_P(str) + 1];
   strcpy_P(c, str);
   printString(c);
}

void printStringLnP(const char *str) {
   printStringP(str);
   Serial.print('\n');
}

void getSonosIP(IPAddress &ip, const char *hostP) {
   char sonosHost[strlen_P(hostP) + 1];
   strcpy_P(sonosHost, hostP);
#if WIFI
   WiFi.hostByName(sonosHost, ip);
#else
   Internet.hostByName(ip, sonosHost);
#endif
}

void connectError() {
   printStringLnP(connError);
}