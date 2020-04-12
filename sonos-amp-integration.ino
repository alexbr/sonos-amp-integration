/********************************************************

  Alex's sonos and amp controller

 *********************************************************/
#include "./Sonos.h"
#include <Ethernet.h>
#include <MicroXPath_P.h>
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

#define PIN_OUT 2 // Serial IR out or 12V op-amp +V(non-inverted) switching
#define NEG_PIN_OUT 3 // For 12V op-amp -V(inverted) switching

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
#define SONOS_STATUS_POLL_DELAY_MS 2000
#define BUTTON_PRESS_VIEW_DURATION_MS 5000
#define AMP_DEBOUNCE_DELAY_MS 2000
#define IR_POWER_DELAY_MS 1000
#define IR_SIZE 67

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
const char httpRequest[] PROGMEM = "HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\n\nGot URI ";

// LCD
const char phonoOverride[] PROGMEM = "Phono override";
const char on[] PROGMEM = "on";
const char off[] PROGMEM = "off";
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

// IR Codes
const unsigned int pwrCode[IR_SIZE] PROGMEM = { 9000, 4500, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560 };
const unsigned int muteCode[IR_SIZE] PROGMEM = { 9000, 4500, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 560, 560 };
const unsigned int volUpCode[IR_SIZE] PROGMEM = { 9000, 4500, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560 };
const unsigned int volDownCode[IR_SIZE] PROGMEM = { 9000, 4500, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560 };
const unsigned int tunerCode[IR_SIZE] PROGMEM = { 9000, 4500, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560 };
const unsigned int phonoCode[IR_SIZE] PROGMEM = { 9000, 4500, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560 };

const byte MAC[] PROGMEM = {0xA8, 0x61, 0x0A, 0xAE, 0x5D, 0x54};

// Living room sonos
const IPAddress livingRoomIP(192, 168, 10, 90);
// Media room sonos
//const IPAddress mediaRoomIP(192, 168, 10, 47);
const IPAddress kitchenIP(192, 168, 10, 78);
const IPAddress sonosIP = livingRoomIP;

EthernetClient sonosClient;
void ethConnectError(){
   char error[strlen_P(ethConnError) + 1];
   strcpy_P(error, ethConnError);
   printStringLn(error);
}
EthernetServer server(80);

Sonos sonos = Sonos(sonosClient, ethConnectError);
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// Sonos
unsigned long lastSonosUpdate;

// LCD
unsigned long displayUntil = 0;
bool clearLcd = false;
uint8_t color = VIOLET;
char row1[LCD_ROW1_LENGTH + 1] = ""; // +1 for '\0'
char row2[LCD_ROW2_LENGTH + 1] = "";
unsigned long lastScrollTime;
uint8_t scrollIndex = -1;

// Amp state
bool ampOn = false;
bool phonoOn = false;
unsigned long offRequestTime = 0; // 0 is "not set"

void setup() {
   Serial.begin(9600);
   while (!Serial) { ; /* needed for native USB */ }

   pinMode(PIN_OUT, OUTPUT);

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
   //printString("IP: " + Ethernet.localIP().toString());

   // set up the LCD's number of columns and rows:
   lcd.begin(16, 2);
}

void loop() {
   checkButtons();
   checkSonos();
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
      if (lastScrollTime > millis() || (millis() - lastScrollTime) > LCD_SCROLL_DELAY_MS) {
         scrollIndex++;
         if (scrollIndex >= rowLen + LCD_SCROLL_PADDING) {
            scrollIndex = 0;
         }
         lastScrollTime = millis();
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
         Serial.println(phonoOn);
         if (phonoOn) {
            phonoOn = false;
            sendIRCode(tunerCode);
            outputAmpOff();
            strcpy_P(row1, phonoOverride);
            strcpy_P(row2, off);
            color = GREEN;
         } else {
            phonoOn = true;
            outputAmpOn();
            sendIRCode(phonoCode);
            strcpy_P(row1, phonoOverride);
            strcpy_P(row2, on);
            color = BLUE;
         }
         /*
         strcpy_P(row1, ip);
         row2[0] = '\0';
         IPAddress localIP = Ethernet.localIP();
         char ipOutput[strlen_P(ipFormat) + 1];
         sprintf(row2, strcpy_P(ipOutput, ipFormat), localIP[0], localIP[1], localIP[2], localIP[3]);
         */
      }

      delay(500);
      clearLcd = true;
      displayUntil = millis() + BUTTON_PRESS_VIEW_DURATION_MS;
   }
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
         if (strcmp_P(uri, muteUri) == 0) {
            sendIRCode(muteCode);
         } else if (strcmp_P(uri, volupUri) >= 0) {
            const uint8_t volSteps = getStepsFromUri(uri);
            for (uint8_t i = 0; i < volSteps; i++) {
               sendIRCode(volUpCode);
               delay(20);
            }
         } else if (strcmp_P(uri, voldownUri) >= 0) {
            const uint8_t volSteps = getStepsFromUri(uri);
            for (uint8_t i = 0; i < volSteps; i++) {
               sendIRCode(volDownCode);
               delay(20);
            }
         } else if (strcmp_P(uri, tunerUri) == 0) {
            outputAmpOn();
            sendIRCode(tunerCode);
            phonoOn = false;
         } else if (strcmp_P(uri, phonoUri) == 0) {
            outputAmpOn();
            sendIRCode(phonoCode);
            phonoOn = true;
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

void checkSonos() {
   // Sonos state polling
   if (!phonoOn &&
         (lastSonosUpdate > millis() ||
         millis() > lastSonosUpdate + SONOS_STATUS_POLL_DELAY_MS)) {
      byte playerState = sonos.getState(sonosIP);
      char uri[20] = "";
      char title[75] = "";
      char artist[40] = "";

      TrackInfo track = sonos.getTrackInfo(sonosIP, uri, sizeof(uri), title, sizeof(title), artist, sizeof(artist));

      byte source = sonos.getSourceFromURI(track.uri);
      char sonosRow1[LCD_ROW1_LENGTH];
      char sonosRow2[LCD_ROW2_LENGTH];
      int sonosColor;

      switch (playerState) {
         case SONOS_STATE_PLAYING:
            strcpy_P(sonosRow1, playing);
            sonosColor = VIOLET;
            outputAmpOn();
            sendIRCode(tunerCode);
            break;
         case SONOS_STATE_PAUSED:
            strcpy_P(sonosRow1, paused);
            sonosColor = YELLOW;
            outputAmpOff();
            break;
         case SONOS_STATE_STOPPED:
            strcpy_P(sonosRow1, stopped);
            sonosColor = RED;
            outputAmpOff();
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
      lastSonosUpdate = millis();
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

void outputAmpOn() {
   offRequestTime = 0;
   if (!ampOn) {
      sendIRCode(pwrCode);
      delay(IR_POWER_DELAY_MS);
      ampOn = true;
   }
}

void outputAmpOff() {
   if (ampOn) {
      // Debounce amp off requests which may occur when sonos switches between
      // play modes
      if (offRequestTime == 0 ||
            millis() - offRequestTime < AMP_DEBOUNCE_DELAY_MS) {
         offRequestTime = millis();
         return;
      }
      offRequestTime = 0;
      sendIRCode(pwrCode);
      ampOn = false;
   }
}

void sendIRCode(int c) {
   int code[IR_SIZE] = {};
   readWords(code, c, IR_SIZE);

   for (int i = 0; i < IR_SIZE; i++) {
      if (i % 2 == 0) { // Set mark
         digitalWrite(PIN_OUT, HIGH);
         delayMicroseconds(code[i]);
      }
      else { // set space
         digitalWrite(PIN_OUT, LOW);
         delayMicroseconds(code[i]);
      }
   }

   digitalWrite(PIN_OUT, LOW);
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
