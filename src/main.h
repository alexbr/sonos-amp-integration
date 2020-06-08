#include <Arduino.h>
#include <Ethernet.h>

// Main loop functions
bool checkButtons();
bool checkServer();
void checkSource();

// Helpers
uint8_t getStepsFromUri(char *uri);
void balOn();
void tunerOn();
void phonoOn();
void phonoOff();
void readBytes(byte *output, const byte *input, const int size);
void readWords(int *output, const int *input, const int size);
void printString(const char *str);
void printStringLn(const char *str);
void getSonosIP(IPAddress &ip, const char *sonosHost);
void connectError();