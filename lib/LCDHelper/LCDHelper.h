#ifndef LCDHelper_h
#define LCDHelper_h

#include <Adafruit_RGBLCDShield.h>

#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

#define LCD_ROW_LENGTH 16
#define LCD_ROW_STR_LENGTH (LCD_ROW_LENGTH + 1)
#define LCD_ROW1_LENGTH LCD_ROW_LENGTH
#define LCD_ROW2_LENGTH 100
#define LCD_SCROLL_DELAY_MS 800
#define LCD_SCROLL_PADDING 15

class LCDHelper {

   public:
      LCDHelper(Adafruit_RGBLCDShield &lcd);

      void printNext(
            const char *row1,
            const char *row2,
            const int color,
            const unsigned long displayUntil);
      void printNextP(
            const char *row1P,
            const char *row2P,
            const int color,
            const unsigned long displayUntil);
      void maybePrintNext(
            const char *row1,
            const char *row2,
            const int color);
      void maybePrintNextP(
            const char *row1P,
            const char *row2P,
            const int color);
      void print();

   private:
      Adafruit_RGBLCDShield *lcd;

      char row1[LCD_ROW1_LENGTH + 1]; // +1 for '\0'
      char row2[LCD_ROW2_LENGTH + 1];
      int color;
      bool clearDisplay;
      bool displayChanged;
      unsigned long displayUntil;
      unsigned long nextScrollTime;
      unsigned int scrollIndex;

      void maybeScrollRow(
            char result[LCD_ROW_STR_LENGTH],
            const char *row);
      void copy16(
            char dest[LCD_ROW_STR_LENGTH],
            const char *src);
};

#endif