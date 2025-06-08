#include <TFT_eSPI.h>
#include "ui.hpp"
#include "characterImage.h"
#include <pgmspace.h>

extern TFT_eSPI tft;

// 前回の値を保存してちらつき防止
static float lastTemp = -999.0;
static float lastSpeed = -999.0;
static bool uiInitialized = false;

void drawUI() {
  if (!uiInitialized) {
    tft.fillScreen(TFT_BLUE);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.setTextSize(3);
    
    // 固定ラベルを一度だけ描画
    tft.drawString("Temp:", 200, 10);
    tft.drawString("Speed:", 200, 130);
    tft.drawString("km/h", 260, 180);
    
    uiInitialized = true;
  }
}

void drawTemperature(float temp) {
  // 値が変わった時のみ更新
  if (abs(temp - lastTemp) > 0.1) {
    // 前の値を消去（背景色で上書き）
    tft.fillRect(200, 35, 120, 25, TFT_BLUE);
    
    // 新しい値を描画
    tft.drawString(String(temp, 1) + " C", 200, 35);
    lastTemp = temp;
  }
}

void drawSpeed(float speed) {
  // 値が変わった時のみ更新
  if (abs(speed - lastSpeed) > 0.1) {
    // 前の値を消去
    tft.fillRect(200, 155, 120, 25, TFT_BLUE);
    
    // 新しい値を描画
    tft.drawString(String(abs(speed), 1), 200, 155);
    lastSpeed = speed;
  }
}

void drawCharacterImage(int x, int y) {
  tft.startWrite();
  tft.setAddrWindow(x, y, 160, 160);
  for (int i = 0; i < 160 * 160; i++) {
    uint16_t color = pgm_read_word(&characterImage[i]);
    tft.pushColor(color);
  }
  tft.endWrite();
}