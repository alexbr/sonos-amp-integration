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
#define LCD_ROW1_LENGTH LCD_ROW_LENGTH
#define LCD_ROW2_LENGTH 100
#define LCD_SCROLL_DELAY_MS 750
#define LCD_SCROLL_PADDING 15

class LCDHelper {

   public:
      LCDHelper(Adafruit_RGBLCDShield lcd);

      void printNext(
            const char *row1,
            const char *row2,
            const int color,
            const unsigned long displayUntil);
      void maybePrintNext(
            const char *row1,
            const char *row2,
            const int color);
      void maybeScrollRow1();
      void maybeScrollRow2();
      void print();

      char row1[LCD_ROW1_LENGTH + 1]; // +1 for '\0'
      char row2[LCD_ROW2_LENGTH + 1];
      int color;
      bool clearDisplay;
      bool displayChanged;

   private:
      Adafruit_RGBLCDShield lcd;
      char displayRow1[LCD_ROW_LENGTH + 1]; // +1 for '\0'
      char displayRow2[LCD_ROW_LENGTH + 1];
      unsigned long displayUntil;
      unsigned long nextScrollTime;
      int scrollIndex;

      void maybeScrollRow(const char *row, char result[LCD_ROW_LENGTH + 1]);
      void LCDHelper::copy16(char *dest, const char *src);
};

#endif
