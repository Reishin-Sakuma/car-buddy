#include <TFT_eSPI.h>
#include "display.hpp"

TFT_eSPI tft = TFT_eSPI();

void initDisplay() {
  tft.init();
  tft.setRotation(1); // 横表示
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
}

void displayTemperature(float tempC) {
  tft.fillRect(0, 0, 240, 40, TFT_BLACK);
  tft.setCursor(10, 10);
  tft.printf("Temp: %.1f C", tempC);
}
