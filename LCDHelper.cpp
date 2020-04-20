#include "LCDHelper.h"
#include <Arduino.h>
#include <Adafruit_RGBLCDShield.h>

LCDHelper::LCDHelper(Adafruit_RGBLCDShield lcd) {
   this->lcd = lcd;
   row1[LCD_ROW1_LENGTH + 1] = "";
   row2[LCD_ROW2_LENGTH + 1] = "";
   color = VIOLET;
   clearDisplay = true;
   displayChanged = true;
   displayUntil = 0;
   displayRow1[LCD_ROW_LENGTH + 1] = "";
   displayRow2[LCD_ROW_LENGTH + 1] = "";
   nextScrollTime = 0;
   scrollIndex = -1;
}

void LCDHelper::printNext(
      const char *row1,
      const char *row2,
      const int color,
      const unsigned long displayUntil) {

   // Just return if nothing has changed
   if (strcmp(this->row1, row1) == 0 && strcmp(this->row2, row2) == 0) {
      displayChanged = false;
      return;
   }

   clearDisplay = true;
   displayChanged = true;
   strcpy(this->row1, row1);
   strcpy(this->row2, row2);
   copy16(displayRow1, this->row1);
   copy16(displayRow2, this->row2);
   this->displayUntil = displayUntil;
   this->color = color;
}

void LCDHelper::maybePrintNext(
      const char *row1,
      const char *row2,
      const int color) {

   if (millis() < this->displayUntil) {
      return;
   }

   printNext(row1, row2, color, 0);
}

void LCDHelper::maybeScrollRow1() {
   maybeScrollRow(row1, displayRow1);
}

void LCDHelper::maybeScrollRow2() {
   maybeScrollRow(row2, displayRow2);
}

void LCDHelper::maybeScrollRow(
      const char *row,
      char result[LCD_ROW_LENGTH + 1]) {

   const int rowLen = strlen(row);

   if (rowLen > LCD_ROW_LENGTH) {
      if (millis() > nextScrollTime) {
         scrollIndex++;
         // We've scrolled beyond all of the row's content plus the padding, so
         // reset the pointer to 0.
         if (scrollIndex >= rowLen + LCD_SCROLL_PADDING) {
            scrollIndex = 0;
         }
         nextScrollTime = millis() + LCD_SCROLL_DELAY_MS;
      } else {
         // Scroll not ready yet, return
         return;
      }

      displayChanged = true;

      /*
      Serial.print("scrollIndex: ");
      Serial.println(scrollIndex);
      */

      for (int i = 0; i < LCD_ROW_LENGTH; i++) {
         unsigned int rowIndex = scrollIndex + i;
         //Serial.print("rowIndex: ");
         //Serial.println(rowIndex);
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
   } else {
      copy16(result, row);
   }
}

void LCDHelper::copy16(char *dest, const char *src) {
   strncpy(dest, src, LCD_ROW_LENGTH + 1);
   dest[min(LCD_ROW_LENGTH, strlen(src))] = '\0';
}

void LCDHelper::print() {
   if (!displayChanged) {
      return;
   }
   displayChanged = false;

   if (clearDisplay) {
      Serial.println("clearing");
      lcd.clear();
      clearDisplay = false;
   }
   /*
   for (int i = 0; i <= strlen(displayRow1); i++) {
      Serial.print(displayRow1[i]);
      Serial.print(',');
   }
   Serial.println();

   for (int i = 0; i <= strlen(displayRow2); i++) {
      Serial.print(displayRow2[i]);
      Serial.print(',');
   }
   Serial.println();
   */

   //Serial.println(strlen(displayRow1));
   Serial.println(displayRow1);

   lcd.setCursor(0, 0);
   lcd.print(displayRow1);

   //Serial.println(strlen(displayRow2));
   //Serial.println(sprintf("%s", displayRow2));
   Serial.println(displayRow2);
   delay(500);

   // No fucking idea what's going on here - the strings are the correct length
   // and terminated properly, but for some reason setting the cursoer to 0, 1
   // starts the second row 5 columns in...
   lcd.setCursor(0, 2);
   lcd.print(displayRow2);
   lcd.setBacklight(color);
}
