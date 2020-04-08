/********************************************************

  Alex's sonos and amp controller

*********************************************************/
#include "./SonosUPnP.h"
#include <Ethernet.h>
#include <MicroXPath_P.h>
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

// #define PWR_CODE 0x7E81542B
// NEC: Raw (68)
//#define PWR_CODE[68] = [4442, 8900, 4350, 600, 550, 600, 1600, 600, 1600, 600, 1600, 600, 1600, 600, 1600, 600, 1600, 600, 550, 550, 1650, 550, 600, 550, 550, 550, 550, 550, 600, 550, 550, 550, 550, 550, 1650, 550, 600, 550, 1600, 600, 550, 550, 1650, 550, 550, 600, 1600, 600, 550, 550, 550, 550, 550, 600, 550, 550, 1600, 600, 550, 550, 1650, 600, 550, 550, 1600, 600, 1600, 600]
//#define TUNER_CODE 0xBE41916E
// NEC: Raw (68)
//#define TUNER_CODE[68] = [776, 8900, 4400, 600, 1600, 600, 550, 550, 1650, 550, 1650, 550, 1650, 550, 1650, 600, 1600, 600, 550, 550, 550, 550, 1650, 550, 600, 550, 550, 550, 550, 550, 550, 600, 550, 550, 1600, 600, 1600, 600, 550, 550, 600, 550, 1600, 600, 550, 550, 550, 600, 550, 550, 1600, 600, 550, 550, 1650, 550, 1650, 600, 550, 550, 1600, 600, 1600, 600, 1600, 600, 550, 550]

#define PIN_OUT 2 // Serial IR out or 12V op-amp +V(non-intervted) switching
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
#define SONOS_STATUS_POLL_DELAY_MS 5000
#define BUTTON_PRESS_VIEW_DURATION_MS 5000

#define ETHERNET_ERROR_DHCP "E: DHCP"
#define ETHERNET_ERROR_CONNECT "E: Connect"

#define SONOS_SPOTIFY_URI_PREFIX "x-sonosprog-spotify:"

const unsigned int pwrCode[67] PROGMEM = { 9000, 4500, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560 };
const unsigned int tunerCode[67] PROGMEM = { 9000, 4500, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 1690, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 560, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690, 560, 560, 560 };

const byte MAC[] PROGMEM = {0xA8, 0x61, 0x0A, 0xAE, 0x5D, 0x54};
const IPAddress staticIP(192, 168, 10, 245);

// Living room sonos
const IPAddress sonosIP(192, 168, 10, 90);
//const char sonosID[] = "949F3E20F930";
// Media room sonos
//const IPAddress sonosIP(192, 168, 10, 47);

EthernetClient ethClient;
void ethConnectError(){
  Serial.println(ETHERNET_ERROR_CONNECT);
}

SonosUPnP sonos = SonosUPnP(ethClient, ethConnectError);
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

unsigned long sonosLastStateUpdate;
unsigned long lastButtonPress;

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; /* needed for native USB */ }
  
  pinMode(PIN_OUT, OUTPUT);

  const byte mac[6] = {};
  readBytes(MAC, mac, 6);

  printString("connecting network...");
  //if (!Ethernet.begin(mac)) {
    printString("starting with MAC failed...");
    Ethernet.begin(mac, staticIP);
  //}
  printString("done");
  printString("IP: ");
  Serial.println(Ethernet.localIP());
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
}

void loop() {
  checkButtons();
  checkSonos();
  displayLcd();
}

// LCD
bool clearLcd = false;
uint8_t color = VIOLET;
char row1[LCD_ROW1_LENGTH + 1] = ""; // + 1 for null terminator
char row2[LCD_ROW2_LENGTH + 1] = "";

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

unsigned long lastScrollTime;
int scrollIndex = -1;

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

      //printString(toPrint);

      lcd.print(toPrint);
   } else {
      lcd.print(row);
   }
}

void checkButtons() {
  uint8_t buttons = lcd.readButtons();
  
  if (buttons) {
    if (buttons & BUTTON_UP) {
      strcpy(row1, "Play");
      row2[0] = '\0';
      color = VIOLET;
      sonos.play(sonosIP);
    }
    if (buttons & BUTTON_DOWN) {
      strcpy(row1, "Pause");
      row2[0] = '\0';
      color = RED;
      sonos.pause(sonosIP);
    }
    if (buttons & BUTTON_LEFT) {
      strcpy(row1, "Previous");
      row2[0] = '\0';
      color = YELLOW;
      sonos.skip(sonosIP, 0); // back
    }
    if (buttons & BUTTON_RIGHT) {
      strcpy(row1, "Next");
      row2[0] = '\0';
      color = TEAL;
      sonos.skip(sonosIP, 1); // forward
    }
    if (buttons & BUTTON_SELECT) {
      strcpy(row1, "Now playing...");
      strcpy(row2, "Fuckit");
      color = BLUE;
    }
    
    clearLcd = true;
    lastButtonPress = millis();
  } 
}

void checkSonos() {
   // Sonos state polling
   if (sonosLastStateUpdate > millis() || millis() > sonosLastStateUpdate + SONOS_STATUS_POLL_DELAY_MS) {
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
            strcpy(sonosRow1, "Playing");
            sonosColor = VIOLET;
            outputAmpOn();
            break;
         case SONOS_STATE_PAUSED:
            strcpy(sonosRow1, "Paused");
            sonosColor = YELLOW;
            outputAmpOff();
            break;
         case SONOS_STATE_STOPPED:
            strcpy(sonosRow1, "Stopped");
            sonosColor = RED;
            outputAmpOff();
            break;
         default:
            strcpy(sonosRow1, "Unknown");
            sonosColor = BLUE;
            break;
      }

      if (source != SONOS_SOURCE_LINEIN) { //source == SONOS_SOURCE_FILE || source == SONOS_SOURCE_HTTP || source == SONOS_SOURCE_RADIO || source == SONOS_SOURCE_MASTER) {
         strcpy(sonosRow2, title);
         if (strlen(artist) > 0 && LCD_ROW2_LENGTH - strlen(sonosRow2) >= 10) {
            strcat(sonosRow2, " by "); 
            strncat(sonosRow2, artist, min(LCD_ROW2_LENGTH - strlen(sonosRow2), strlen(artist)));
         }
      }

      maybePrintSonosUpdate(sonosRow1, sonosRow2, sonosColor);
      sonosLastStateUpdate = millis();
   }
}

void maybePrintSonosUpdate(const char *sonosRow1, const char *sonosRow2, int sonosColor) {
   if (lastButtonPress > millis() || millis() > lastButtonPress + BUTTON_PRESS_VIEW_DURATION_MS) {
      strcpy(row1, sonosRow1);
      strcpy(row2, sonosRow2);
      color = sonosColor;
      clearLcd = true;
   }
}

bool ampOn = false;
unsigned long offRequestTime = 0; // 0 is "not set"

void outputAmpOn() {
   offRequestTime = 0;
   if (!ampOn) { 
      printString("turning on");
      sendIRCode(pwrCode, 67);
      delay(2000);
      printString("setting tuner");
      sendIRCode(tunerCode, 67);
      ampOn = true;
   }
}


void outputAmpOff() {
   if (ampOn) {
      // Debounce amp off requests which may occur when sonos switches between play modes
      if (offRequestTime == 0 || millis() - offRequestTime < 2000) {
         offRequestTime = millis();
         return;
      }
      offRequestTime = 0; 
      printString("turning off");
      sendIRCode(pwrCode, 67);
      ampOn = false;
   }
}

void sendIRCode(int c, int size) {
   int code[size] = {};
   readWords(c, code, size);

   printString("sending IR code");
   for (int i = 0; i < size; i++) {
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

   delay(2000);
}

void readBytes(byte input[], byte output[], int size) {
  for (int i = 0; i < size; i++) {
    output[i] = pgm_read_byte_near(input + i);
  }
}

int readWords(int input[], int output[], int size) {
  for (int i = 0; i < size; i++) {
    output[i] = pgm_read_word_near(input + i);
  }
}

/**
 * String helpers
 */
void printString(const char *str) {
   const char *p = str;
   while (*p) {
      Serial.print(*p);
      p++;
   }
   Serial.print('\n');
}

// adds string 2 to the end of string 1
void concatString(const char *str1, const char *str2, const char *output) {
    strcat(str1,  str2);
    strcpy(output, str1); // copy string to outputstring
    return output;
}

void replaceInString(char *dest, const char *source, const char *toReplace, const char *replaceWith) {
   int matchIndex = 0;
   for (int i = 0; i < strlen(source); i++) {
      const char c = source[i];
      if (c == toReplace[matchIndex]) {
         if (matchIndex == strlen(toReplace)) {
            matchIndex = 0;
            // replace
         } else {
         }
      }
   }
}
