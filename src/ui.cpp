#include <TFT_eSPI.h>
#include "ui.hpp"
#include "characterImage.h"
#include <pgmspace.h>

extern TFT_eSPI tft;  // main.cppで定義されたインスタンスを使う想定

void drawUI() {
  tft.fillScreen(TFT_BLUE);

  // フォント設定
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(3);
}

void drawSpeed(float speed) {
  // 速度表示（右下）
  tft.drawString("Speed: " , 200, 130);
  tft.drawString(String(speed, 1), 200, 155);
  tft.drawString(" km/h" , 200, 180);
}

void drawTemperature(float temp) {
  tft.drawString("Temp:", 200, 10);               // 1行目
  tft.drawString(String(temp, 1) + " C", 200, 35); // 2行目（Y座標をずらす）
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