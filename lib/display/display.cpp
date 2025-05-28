#include "display.h"
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();  // TFT_eSPI は内部で SPI.begin() を呼ぶ

void initDisplay() {
  tft.init();
  tft.setRotation(1);  // 横向き（Landscape）
  tft.fillScreen(TFT_BLACK);
  drawUILayout();
}

void drawUILayout() {
  // レイアウト枠線
  tft.drawRect(0, 0, 160, 240, TFT_WHITE);         // 左：キャラクター表示エリア
  tft.drawRect(160, 0, 160, 120, TFT_BLUE);        // 右上：温度表示エリア
  tft.drawRect(160, 120, 160, 120, TFT_GREEN);     // 右下：速度表示エリア
}

void drawTemperature(float temp) {
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(2);
  tft.setCursor(170, 30);
  tft.fillRect(160, 0, 160, 120, TFT_BLUE);  // 表示更新前に領域を塗りつぶし
  tft.setCursor(170, 30);
  tft.printf("温度: %.1f C", temp);
}

void drawSpeed(int spd) {
  tft.setTextColor(TFT_WHITE, TFT_GREEN);
  tft.setTextSize(2);
  tft.fillRect(160, 120, 160, 120, TFT_GREEN);
  tft.setCursor(170, 160);
  tft.printf("速度: %d km/h", spd);
}
