#ifndef main_h_
#define main_h_

#include <Arduino.h>

#define TRIGGER_PIN_OUT 2 // For 12V switching
#define IR_PIN_OUT 3      // Serial IR out

#define SOURCE_STATUS_POLL_DELAY_MS 3000
#define BUTTON_PRESS_VIEW_DURATION_MS 5000
#define CHECK_TIME_DELAY_MS 300000
#define WIFI_CONNECT_TIMEOUT_MS 20000  
#define WIFI_CONNECT_TRIES 2   
#define WIFI_RESET_TIMEOUT_MS 30000  
#define PING_DELAY_MS 500

// Main loop functions
bool checkButtons();
bool checkServer();
void checkSource();
bool checkConnectionStatus(bool forcePing);

// Helpers
bool connect();
uint8_t getStepsFromUri(char *uri);
void balOn();
void tunerOn();
void phonoOn();
void phonoOff();
unsigned long getTime();
void readBytes(byte *output, const byte *input, const int size);
void readWords(int *output, const int *input, const int size);
void printString(const char *str);
void printStringLn(const char *str);
void printStringP(const char *str);
void printStringLnP(const char *str);
void getSonosIP(IPAddress &ip, const char *sonosHost);
void connectError();

#endif