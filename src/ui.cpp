#include <TFT_eSPI.h>
#include "ui.hpp"
#include "characterImage.h"
#include <pgmspace.h>

extern TFT_eSPI tft;

// 前回の値を保存してちらつき防止
static float lastTemp = -999.0;
static float lastSpeed = -999.0;
static bool uiInitialized = false;

// スプラッシュ画面を表示
void showSplashScreen() {
  // 基本設定
  String title = "car-buddy";
  String subtitle = "Talking Monitor v1.0";
  int titleX = (320 - title.length() * 24) / 2;
  int titleY = (240 - 32) / 2;
  int subX = (320 - subtitle.length() * 12) / 2;
  int subY = titleY + 50;
  
  // フェードイン（16段階でより滑らか）
  for (int fade = 0; fade <= 15; fade++) {
    tft.fillScreen(TFT_BLACK);
    
    // より広い範囲でフェード効果（0-255の範囲を使用）
    uint8_t brightness = fade * 17;  // 0, 17, 34, ..., 255
    uint16_t fadeColor = tft.color565(brightness, brightness, brightness);
    tft.setTextColor(fadeColor, TFT_BLACK);
    
    // タイトル描画
    tft.setTextSize(4);
    tft.drawString(title, titleX, titleY);
    
    // サブタイトル描画
    tft.setTextSize(2);
    tft.drawString(subtitle, subX, subY);
    
    delay(150);  // フェード速度を少し遅く
  }
  
  // 完全表示（白色で確実に表示）
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(4);
  tft.drawString(title, titleX, titleY);
  tft.setTextSize(2);
  tft.drawString(subtitle, subX, subY);
  
  delay(1500);  // 完全表示時間を少し長く
  
  // フェードアウト（16段階）
  for (int fade = 15; fade >= 0; fade--) {
    tft.fillScreen(TFT_BLACK);
    
    // フェードアウト効果
    uint8_t brightness = fade * 17;
    uint16_t fadeColor = tft.color565(brightness, brightness, brightness);
    tft.setTextColor(fadeColor, TFT_BLACK);
    
    // タイトル描画
    tft.setTextSize(4);
    tft.drawString(title, titleX, titleY);
    
    // サブタイトル描画
    tft.setTextSize(2);
    tft.drawString(subtitle, subX, subY);
    
    delay(120);  // フェードアウト速度
  }
  
  // 最終的に黒画面
  tft.fillScreen(TFT_BLACK);
  delay(300);
}

void drawUI() {
  if (!uiInitialized) {
    // メイン画面のフェードイン
    fadeInMainScreen();
    uiInitialized = true;
  }
}

// メイン画面をフェードインで表示
void fadeInMainScreen() {
  // フェードイン（8段階）
  for (int fade = 0; fade <= 7; fade++) {
    // 背景色のフェード
    uint16_t bgColor = tft.color565(0, 0, fade * 32);  // 青色のフェード
    tft.fillScreen(bgColor);
    
    // テキスト色のフェード
    uint16_t textColor = tft.color565(fade * 32, fade * 32, fade * 32);
    tft.setTextColor(textColor, bgColor);
    tft.setTextSize(3);
    
    // 固定ラベルを描画
    tft.drawString("Temp:", 200, 10);
    tft.drawString("Speed:", 200, 130);
    tft.drawString("km/h", 240, 180);
    
    delay(80);  // フェード速度
  }
  
  // 最終的に完全な色で描画
  tft.fillScreen(TFT_BLUE);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(3);
  tft.drawString("Temp:", 200, 10);
  tft.drawString("Speed:", 200, 130);
  tft.drawString("km/h", 240, 180);
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

// キャラクター画像をフェードインで表示
void drawCharacterImageWithFade(int x, int y) {
  const int originalSize = 160;
  const int newSize = 180;
  const float scale = (float)newSize / originalSize;
  
  // フェードイン（8段階）
  for (int fade = 0; fade <= 7; fade++) {
    tft.startWrite();
    tft.setAddrWindow(x, y, newSize, newSize);
    
    for (int row = 0; row < newSize; row++) {
      for (int col = 0; col < newSize; col++) {
        int srcRow = (int)(row / scale);
        int srcCol = (int)(col / scale);
        
        if (srcRow >= originalSize) srcRow = originalSize - 1;
        if (srcCol >= originalSize) srcCol = originalSize - 1;
        
        int srcIndex = srcRow * originalSize + srcCol;
        uint16_t originalColor = pgm_read_word(&characterImage[srcIndex]);
        
        // 色をフェード処理
        uint8_t r = ((originalColor >> 11) & 0x1F) * fade / 7;
        uint8_t g = ((originalColor >> 5) & 0x3F) * fade / 7;
        uint8_t b = (originalColor & 0x1F) * fade / 7;
        uint16_t fadeColor = tft.color565(r << 3, g << 2, b << 3);
        
        tft.pushColor(fadeColor);
      }
    }
    
    tft.endWrite();
    delay(60);  // キャラクターフェード速度
  }
}

// 通常のキャラクター画像表示（フェードなし）
void drawCharacterImage(int x, int y) {
  const int originalSize = 160;
  const int newSize = 180;
  const float scale = (float)newSize / originalSize;
  
  tft.startWrite();
  tft.setAddrWindow(x, y, newSize, newSize);
  
  for (int row = 0; row < newSize; row++) {
    for (int col = 0; col < newSize; col++) {
      int srcRow = (int)(row / scale);
      int srcCol = (int)(col / scale);
      
      if (srcRow >= originalSize) srcRow = originalSize - 1;
      if (srcCol >= originalSize) srcCol = originalSize - 1;
      
      int srcIndex = srcRow * originalSize + srcCol;
      uint16_t color = pgm_read_word(&characterImage[srcIndex]);
      
      tft.pushColor(color);
    }
  }
  
  tft.endWrite();
}