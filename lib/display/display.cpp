#include <TFT_eSPI.h>
#include "display.h"
TFT_eSPI tft = TFT_eSPI();

void initDisplay() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
}

void showBootMessage() {
  tft.drawString("起動中...", 10, 10);
}

void showTemperature(float temp) {
  tft.fillRect(10, 60, 200, 30, TFT_BLACK);
  tft.setTextColor(TFT_YELLOW);
  tft.drawString("温度: " + String(temp, 1) + " C", 10, 60);
}