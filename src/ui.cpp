#include <TFT_eSPI.h>
#include "ui.hpp"

extern TFT_eSPI tft;  // main.cppで定義されたインスタンスを使う想定

void drawUI(float temperature, float speed) {
  tft.fillScreen(TFT_BLUE);

  // フォント設定
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(3);

  // 温度表示（右上）
  tft.drawString("Temp:\n" + String(temperature, 1) + " C", 200, 10);

  // 速度表示（右下）
  tft.drawString("Speed:\n" + String(speed, 1) + " km/h", 200, 150);

  // キャラ画像枠（仮）
  tft.drawRect(10, 20, 160, 200, TFT_WHITE);
}
