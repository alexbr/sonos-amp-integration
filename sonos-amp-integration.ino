/********************************************************

  Alex's sonos and amp controller

 *********************************************************/
#include "./Sonos.h"
#include "./AmpControl.h"
#include <Ethernet.h>
#include <MicroXPath_P.h>
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

#define IR_PIN_OUT 2 // Serial IR out
#define TRIGGER_PIN_OUT 3 // For 12V switching

#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

#define LCD_ROW1_LENGTH 16
#define LCD_ROW2_LENGTH 100
#define LCD_SCROLL_DELAY_MS 500
#define LCD_SCROLL_PADDING 15
#define SOURCE_STATUS_POLL_DELAY_MS 2000
#define BUTTON_PRESS_VIEW_DURATION_MS 5000

// Ethernet
const char ethConnError[] PROGMEM = "Connect error";
const char ipFormat[] PROGMEM = "%d.%d.%d.%d";

// HTTP Server
const char httpGet[] PROGMEM = "GET /";
const char muteUri[] PROGMEM = "mute";
const char volupUri[] PROGMEM = "volup";
const char voldownUri[] PROGMEM = "voldown";
const char tunerUri[] PROGMEM = "tuner";
const char phonoUri[] PROGMEM = "phono";
const char pwrOnUri[] PROGMEM = "pwron";
const char pwrOffUri[] PROGMEM = "pwroff";
const char httpRequest[] PROGMEM = "HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\n\nGot URI ";

// LCD
const char phonoOverride1[] PROGMEM = "Record player";
const char phonoOn2[] PROGMEM = "on.";
const char phonoOff2[] PROGMEM = "off.";
const char playing[] PROGMEM = "Playing";
const char paused[] PROGMEM = "Paused";
const char stopped[] PROGMEM = "Stopped";
const char unknown[] PROGMEM = "Unknown";
const char by[] PROGMEM = " by ";

const char play[] PROGMEM = "Play";
const char pause[] PROGMEM = "Pause";
const char previous[] PROGMEM = "Previous";
const char next[] PROGMEM = "Next";
const char ip[] PROGMEM = "IP";

// Ethernet setup
const byte MAC[] PROGMEM = { 0xA8, 0x61, 0x0A, 0xAE, 0x5D, 0x54 };
EthernetClient sonosClient;
void ethConnectError(){
   char error[strlen_P(ethConnError) + 1];
   strcpy_P(error, ethConnError);
   printStringLn(error);
}
EthernetServer server(80);

// Amp control
AmpControl amp = AmpControl(IR_PIN_OUT, TRIGGER_PIN_OUT);
char intendedSource = SRC_UNKNOWN;
unsigned long checkSourceAfter = 0;

// Sonos setup
// Living room sonos
const IPAddress livingRoomIP(192, 168, 10, 90);
// Media room sonos
//const IPAddress mediaRoomIP(192, 168, 10, 47);
const IPAddress kitchenIP(192, 168, 10, 78);
const IPAddress sonosIP = livingRoomIP;
Sonos sonos = Sonos(sonosClient, ethConnectError);

// LCD
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
unsigned long displayUntil = 0;
bool clearLcd = false;
uint8_t color = VIOLET;
char row1[LCD_ROW1_LENGTH + 1] = ""; // +1 for '\0'
char row2[LCD_ROW2_LENGTH + 1] = "";
unsigned long nextScrollTime = 0;
uint8_t scrollIndex = -1;

void setup() {
   Serial.begin(9600);
   while (!Serial) { ; /* needed for native USB */ }

   pinMode(IR_PIN_OUT, OUTPUT);
   pinMode(TRIGGER_PIN_OUT, OUTPUT);

   const byte mac[6] = {};
   readBytes(mac, MAC, 6);

   //printStringLn("connecting ethernet...");
   if (!Ethernet.begin(mac)) {
      const IPAddress staticIP(192, 168, 10, 245);
      //printStringLn("starting with MAC failed...");
      Ethernet.begin(mac, staticIP);
   }
   server.begin();
   //printStringLn("ethernet initialized");
   //printString("IP: " + Ethernet.localIP());

   // set up the LCD's number of columns and rows:
   lcd.begin(16, 2);
}

void loop() {
   checkButtons();
   checkSource();
   checkServer();
   displayLcd();
}

void displayLcd() {
   if (clearLcd) {
      lcd.clear();
      clearLcd = false;
   }

   lcd.setCursor(0, 0);
   maybeScrollLcdRow(row1);
   lcd.setCursor(0, 1);
   maybeScrollLcdRow(row2);
   lcd.setBacklight(color);
}

void maybeScrollLcdRow(const char *row) {
   const int rowLen = strlen(row);

   if (rowLen > 16) {
      if (millis() > nextScrollTime) {
         scrollIndex++;
         if (scrollIndex >= rowLen + LCD_SCROLL_PADDING) {
            scrollIndex = 0;
         }
         nextScrollTime = millis() + LCD_SCROLL_DELAY_MS;
      }

      char toPrint[17];
      for (int i = 0; i < 16; i++) {
         unsigned int rowIndex = scrollIndex + i;
         unsigned char c;
         if (rowIndex >= rowLen) {
            if (rowIndex < rowLen + LCD_SCROLL_PADDING) {
               c = ' ';
            } else {
               c = row[rowIndex - rowLen - LCD_SCROLL_PADDING];
            }
         } else {
            c = row[rowIndex];
         }
         toPrint[i] = c;
      }

      toPrint[16] = '\0';

      //printStringLn(toPrint);

      lcd.print(toPrint);
   } else {
      lcd.print(row);
   }
}

void checkButtons() {
   uint8_t buttons = lcd.readButtons();

   if (buttons) {
      if (buttons & BUTTON_UP) {
         strcpy_P(row1, play);
         row2[0] = '\0';
         color = VIOLET;
         sonos.play(sonosIP);
      }
      if (buttons & BUTTON_DOWN) {
         strcpy_P(row1, pause);
         row2[0] = '\0';
         color = RED;
         sonos.pause(sonosIP);
      }
      if (buttons & BUTTON_LEFT) {
         strcpy_P(row1, previous);
         row2[0] = '\0';
         color = YELLOW;
         sonos.skip(sonosIP, 0); // back
      }
      if (buttons & BUTTON_RIGHT) {
         strcpy_P(row1, next);
         row2[0] = '\0';
         color = TEAL;
         sonos.skip(sonosIP, 1); // forward
      }
      if (buttons & BUTTON_SELECT) {
         if (intendedSource == SRC_PHONO) {
            phonoOff();
         } else {
            phonoOn();
         }
         /*
         strcpy_P(row1, ip);
         row2[0] = '\0';
         IPAddress localIP = Ethernet.localIP();
         char ipOutput[strlen_P(ipFormat) + 1];
         sprintf(row2, strcpy_P(ipOutput, ipFormat), localIP[0], localIP[1], localIP[2], localIP[3]);
         */
      }

      // Debounce button press
      delay(200);
      clearLcd = true;
      displayUntil = millis() + BUTTON_PRESS_VIEW_DURATION_MS;
   }
}

// TODO: rate limit any IR commands
void tunerOn() {
   intendedSource = SRC_TUNER;
   amp.turnOn();
   amp.tuner();
}

void phonoOn() {
   intendedSource = SRC_PHONO;
   amp.turnOn();
   amp.phono();
   strcpy_P(row1, phonoOverride1);
   strcpy_P(row2, phonoOn2);
   color = BLUE;
   clearLcd = true;
}

void phonoOff() {
   intendedSource = SRC_UNKNOWN;
   amp.turnOffWithDebounce();
   strcpy_P(row1, phonoOverride1);
   strcpy_P(row2, phonoOff2);
   color = TEAL;
   clearLcd = true;
}

void checkServer() {
   EthernetClient client = server.available();

   if (client) {
      char get[strlen_P(httpGet) + 1];
      strcpy_P(get, httpGet);
      char uri[16] = "";
      int index = 0; // used for method and URI indexing

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
         //Serial.println(uri);
         if (strcmp_P(uri, muteUri) == 0) {
            amp.mute();
         } else if (strcmp_P(uri, volupUri) >= 0) {
            const uint8_t volSteps = getStepsFromUri(uri);
            for (uint8_t i = 0; i < volSteps; i++) {
               amp.volumeUp();
               delay(20);
            }
         } else if (strcmp_P(uri, voldownUri) >= 0) {
            const uint8_t volSteps = getStepsFromUri(uri);
            for (uint8_t i = 0; i < volSteps; i++) {
               amp.volumeDown();
               delay(20);
            }
         } else if (strcmp_P(uri, tunerUri) == 0) {
            tunerOn();
         } else if (strcmp_P(uri, phonoUri) == 0) {
            phonoOn();
         } else if (strcmp_P(uri, pwrOnUri) == 0) {
            amp.turnOn();
         } else if (strcmp_P(uri, pwrOffUri) == 0) {
            amp.turnOff();
         }
      }
   }

   delay(1);
   client.stop();
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

void checkSource() {
   if (millis() < checkSourceAfter) {
      return;
   }

   checkSourceAfter = millis() + SOURCE_STATUS_POLL_DELAY_MS;

   if (intendedSource == SRC_PHONO) {
      amp.turnOn();
      amp.phono();
   } else {
      // Sonos state polling
      byte playerState = sonos.getState(sonosIP);
      char uri[20] = "";
      char title[75] = "";
      char artist[40] = "";

      TrackInfo track = sonos.getTrackInfo(
            sonosIP,
            uri,
            sizeof(uri),
            title,
            sizeof(title),
            artist,
            sizeof(artist));

      byte source = sonos.getSourceFromURI(track.uri);
      char sonosRow1[LCD_ROW1_LENGTH];
      char sonosRow2[LCD_ROW2_LENGTH];
      int sonosColor;

      switch (playerState) {
         case SONOS_STATE_PLAYING:
            strcpy_P(sonosRow1, playing);
            sonosColor = VIOLET;
            tunerOn();
            break;
         case SONOS_STATE_PAUSED:
            strcpy_P(sonosRow1, paused);
            sonosColor = RED;
            amp.turnOffWithDebounce();
            break;
         case SONOS_STATE_STOPPED:
            strcpy_P(sonosRow1, stopped);
            sonosColor = YELLOW;
            amp.turnOffWithDebounce();
            break;
         default:
            strcpy_P(sonosRow1, unknown);
            sonosColor = BLUE;
            break;
      }

      if (source == SONOS_SOURCE_MASTER) {
         if (sonosIP == livingRoomIP) {
            sonosIP = kitchenIP;
         } else {
            sonosIP = livingRoomIP;
         }
      }

      if (source != SONOS_SOURCE_LINEIN) {
         strcpy(sonosRow2, title);
         if (strlen(artist) > 0 && LCD_ROW2_LENGTH - strlen(sonosRow2) >= 10) {
            strcat_P(sonosRow2, by);
            strncat(sonosRow2, artist, min(LCD_ROW2_LENGTH - strlen(sonosRow2), strlen(artist)));
         }
      }

      maybePrintSonosUpdate(sonosRow1, sonosRow2, sonosColor);
   }
}

void maybePrintSonosUpdate(const char *sonosRow1, const char *sonosRow2, int sonosColor) {
   if (millis() > displayUntil) {
      strcpy(row1, sonosRow1);
      strcpy(row2, sonosRow2);
      color = sonosColor;
      clearLcd = true;
   }
}

void readBytes(byte output[], const byte input[], const int size) {
   for (int i = 0; i < size; i++) {
      output[i] = pgm_read_byte_near(input + i);
   }
}

int readWords(int output[], const int input[], const int size) {
   for (int i = 0; i < size; i++) {
      output[i] = pgm_read_word_near(input + i);
   }
}

/**
 * String helpers
 */
void printStringLn(const char *str) {
   printString(str);
   Serial.print('\n');
}

void printString(const char *str) {
   const char *p = str;
   while (*p) {
      Serial.print(*p);
      p++;
   }
}
