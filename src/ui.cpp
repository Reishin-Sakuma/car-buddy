#include <TFT_eSPI.h>
#include "ui.hpp"
#include "characterImage.h"
#include <pgmspace.h>

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
  tft.drawRect(10, 20, 160, 160, TFT_WHITE);
}

void drawCharacterImage(int x, int y) {
  // 背景をクリア（黒で上書き）
//   tft.fillRect(x, y, 160, 160, TFT_BLACK);
  tft.startWrite();
  tft.setAddrWindow(x, y, 160, 160); // サイズに応じて変更
  for (int i = 0; i < 160 * 160; i++) {
    uint16_t color = pgm_read_word(&characterImage[i]);
    tft.pushColor(color);
  }
  tft.endWrite();
}