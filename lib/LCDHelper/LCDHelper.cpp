#include "LCDHelper.h"
#include <Arduino.h>
#include <Adafruit_RGBLCDShield.h>

LCDHelper::LCDHelper(Adafruit_RGBLCDShield &lcd) {
   this->lcd = &lcd;
   this->color = VIOLET;
   this->clearDisplay = true;
   this->displayUntil = 0;
   this->nextScrollTime = 0;
   this->scrollIndex = 0;
}

void LCDHelper::printNext(
      const char *row1,
      const char *row2,
      const int color,
      const unsigned long displayUntil) {

   this->displayUntil = displayUntil;
   this->color = color;

   // Just return if the text hasn't changed
   if (strcmp(this->row1, row1) == 0 && strcmp(this->row2, row2) == 0) {
      return;
   }

   strcpy(this->row1, row1);
   strcpy(this->row2, row2);
   this->clearDisplay = true;
}

void LCDHelper::printNextP(
      const char *row1P,
      const char *row2P,
      const int color,
      const unsigned long displayUntil) {
   char row1[strlen_P(row1P) + 1];
   char row2[strlen_P(row2P) + 1];
   strcpy_P(this->row1, row1P);
   strcpy_P(this->row2, row2P);
   printNext(this->row1, this->row2, color, 0);
}

void LCDHelper::maybePrintNext(
      const char *row1,
      const char *row2,
      const int color) {

   if (millis() < displayUntil) {
      return;
   }

   printNext(row1, row2, color, 0);
}

void LCDHelper::maybePrintNextP(
      const char *row1P,
      const char *row2P,
      const int color) {
   char row1[strlen_P(row1P) + 1];
   char row2[strlen_P(row2P) + 1];
   strcpy_P(row1, row1P);
   strcpy_P(row2, row2P);
   maybePrintNext(row1, row2, color);
}

void LCDHelper::maybeScrollRow(
      char result[LCD_ROW_STR_LENGTH],
      const char *row) {
   const unsigned int rowLen = strlen(row);

   // No scroll necessary, copy and return
   if (rowLen < LCD_ROW_LENGTH) {
      copy16(result, row);
      return;
   }

   if (millis() >= this->nextScrollTime) {
      this->nextScrollTime = millis() + LCD_SCROLL_DELAY_MS;
      this->scrollIndex++;

      // We've scrolled beyond all of the row's content plus the padding, so
      // reset the pointer to 0.
      if (this->scrollIndex >= rowLen + LCD_SCROLL_PADDING) {
         this->scrollIndex = 0;
      }
   }

   for (int i = 0; i < LCD_ROW_LENGTH; i++) {
      unsigned int rowIndex = this->scrollIndex + i;
      unsigned char c;
      if (rowIndex >= rowLen) {
         // This is the padding between the last char of the content and the
         // start of the beginning of the content entering from the right of
         // the screen.
         if (rowIndex < rowLen + LCD_SCROLL_PADDING) {
            c = ' ';
         } else {
            // This is the beginning of the content entering from the right
            // of the screen.
            c = row[rowIndex - rowLen - LCD_SCROLL_PADDING];
         }
      } else {
         c = row[rowIndex];
      }

      result[i] = c;
   }

   result[LCD_ROW_LENGTH] = '\0';
}

void LCDHelper::copy16(
      char dest[LCD_ROW_STR_LENGTH],
      const char *src) {
   strncpy(dest, src, LCD_ROW_LENGTH);
   dest[min(LCD_ROW_LENGTH, strlen(src))] = '\0';
}

void LCDHelper::print() {
   if (this->clearDisplay) {
      lcd->clear();
      // Restart scrolling
      this->scrollIndex = -1;
      clearDisplay = false;
   }

   char row[LCD_ROW_STR_LENGTH];
   maybeScrollRow(row, this->row1);
   lcd->setCursor(0, 0);
   lcd->print(row);

   maybeScrollRow(row, this->row2);
   lcd->setCursor(0, 1);
   lcd->print(row);

   lcd->setBacklight(this->color);
}

void LCDHelper::screenOff() {
   this->lcd->clear();
   this->lcd->setBacklight(BLACK);
   this->lcd->noDisplay();
}

void LCDHelper::screenOn() {
   this->lcd->display();
}