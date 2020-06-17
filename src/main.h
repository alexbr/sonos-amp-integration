#ifndef main_h_
#define main_h_
#include <Arduino.h>

// Main loop functions
bool checkButtons();
bool checkServer();
void checkSource();
void checkConnection();

// Helpers
void connect();
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
void getSonosIP(IPAddress &ip, const char *sonosHost);
void connectError();

#endif