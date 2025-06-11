#include <TFT_eSPI.h>
#include "ui.hpp"
#include "characterImage.h"
#include <pgmspace.h>

extern TFT_eSPI tft;

// 前回の値を保存してちらつき防止
static float lastTemp = -999.0;
static float lastSpeed = -999.0;
static bool uiInitialized = false;

// 時刻表示用の静的変数
static String lastDisplayTime = "";
static String lastDisplayDate = "";

// ===== グラデーション関数 =====

// 基本的なグラデーション背景を描画（上：濃い青 → 下：明るい青）
void drawGradientBackground() {
  for (int y = 0; y < 240; y++) {
    // グラデーション計算（0.0から1.0の範囲）
    float ratio = (float)y / 240.0;
    
    // 濃い青(0, 0, 128) から 明るい青(64, 128, 255) へのグラデーション
    uint8_t r = (uint8_t)(0 + (64 * ratio));
    uint8_t g = (uint8_t)(0 + (128 * ratio));
    uint8_t b = (uint8_t)(128 + (127 * ratio));
    
    uint16_t lineColor = tft.color565(r, g, b);
    
    // 横一列を描画
    tft.drawFastHLine(0, y, 320, lineColor);
  }
}

// 特定エリアのグラデーション背景を再描画（文字消去用）
void drawGradientArea(int x, int y, int width, int height) {
  for (int row = y; row < y + height; row++) {
    if (row >= 240) break;  // 画面外チェック
    
    float ratio = (float)row / 240.0;
    uint8_t r = (uint8_t)(0 + (64 * ratio));
    uint8_t g = (uint8_t)(0 + (128 * ratio));
    uint8_t b = (uint8_t)(128 + (127 * ratio));
    uint16_t lineColor = tft.color565(r, g, b);
    
    tft.drawFastHLine(x, row, width, lineColor);
  }
}

// ===== 既存関数（グラデーション対応版） =====

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

// メイン画面をフェードインで表示（グラデーション対応）
void fadeInMainScreen() {
  // フェードイン（8段階）
  for (int fade = 0; fade <= 7; fade++) {
    // グラデーション背景を描画
    drawGradientBackground();
    
    // フェード効果のためのオーバーレイ
    if (fade < 7) {
      uint8_t fadeAlpha = 255 - (fade * 32);
      uint16_t fadeOverlay = tft.color565(fadeAlpha / 8, fadeAlpha / 8, fadeAlpha / 4);
      
      // 半透明オーバーレイでフェード効果（軽量化版）
      for (int y = 0; y < 240; y += 3) {
        for (int x = 0; x < 320; x += 3) {
          if ((x + y) % 6 == 0) {  // まばらなパターンで軽量化
            tft.drawPixel(x, y, fadeOverlay);
          }
        }
      }
    }
    
    // テキスト色のフェード（白文字で視認性重視）
    uint16_t textColor = tft.color565(fade * 32, fade * 32, fade * 32);
    tft.setTextColor(textColor);
    tft.setTextSize(3);
    
    // 固定ラベルを描画
    tft.drawString("Temp:", 200, 10);
    tft.drawString("Speed:", 200, 130);
    tft.drawString("km/h", 240, 180);
    
    delay(80);  // フェード速度
  }
  
  // 最終的に完全なグラデーション背景で描画
  drawGradientBackground();
  
  // 最終テキスト描画（白文字で見やすく）
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.drawString("Temp:", 200, 10);
  tft.drawString("Speed:", 200, 130);
  tft.drawString("km/h", 240, 180);
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

// キャラクター画像を縁フェード付きで表示（グラデーション背景対応）
void drawCharacterImageWithEdgeFade(int x, int y) {
  const int originalSize = 160;
  const int newSize = 180;
  const float scale = (float)newSize / originalSize;
  const int fadeWidth = 8;  // フェード幅を小さく（弱め）
  
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
      
      // 四角い縁からの距離を計算
      int distFromEdge = min(min(row, col), min(newSize - 1 - row, newSize - 1 - col));
      
      if (distFromEdge < fadeWidth) {
        // フェード領域：グラデーション背景色と軽くブレンド
        float alpha = 0.3 + 0.7 * ((float)distFromEdge / fadeWidth);  // 0.3-1.0の範囲（弱め）
        
        // 元の色を分解
        uint8_t r = (originalColor >> 11) & 0x1F;
        uint8_t g = (originalColor >> 5) & 0x3F;
        uint8_t b = originalColor & 0x1F;
        
        // グラデーション背景色を計算（その位置での背景色）
        float bgY = (float)(y + row) / 240.0;
        uint8_t bgR = (uint8_t)(0 + (64 * bgY)) >> 3;  // 5ビットに変換
        uint8_t bgG = (uint8_t)(0 + (128 * bgY)) >> 2; // 6ビットに変換
        uint8_t bgB = (uint8_t)(128 + (127 * bgY)) >> 3; // 5ビットに変換
        
        // 軽いアルファブレンド
        uint8_t blendR = (uint8_t)(r * alpha + bgR * (1.0 - alpha));
        uint8_t blendG = (uint8_t)(g * alpha + bgG * (1.0 - alpha));
        uint8_t blendB = (uint8_t)(b * alpha + bgB * (1.0 - alpha));
        
        uint16_t fadeColor = (blendR << 11) | (blendG << 5) | blendB;
        tft.pushColor(fadeColor);
      } else {
        // 通常領域：元の色をそのまま
        tft.pushColor(originalColor);
      }
    }
  }
  
  tft.endWrite();
}

// より高品質な縁フェード（ガウシアンブラー風）
void drawCharacterImageWithSoftEdge(int x, int y) {
  const int originalSize = 160;
  const int newSize = 180;
  const float scale = (float)newSize / originalSize;
  const int fadeWidth = 20;  // フェード幅
  
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
      
      // 中心からの距離でソフトな円形フェード
      float centerX = newSize / 2.0;
      float centerY = newSize / 2.0;
      float distFromCenter = sqrt((row - centerY) * (row - centerY) + (col - centerX) * (col - centerX));
      float maxRadius = newSize / 2.0;
      float fadeRadius = maxRadius - fadeWidth;
      
      if (distFromCenter > fadeRadius) {
        // フェード領域：円形グラデーション
        float alpha = 1.0 - (distFromCenter - fadeRadius) / fadeWidth;
        if (alpha < 0) alpha = 0;
        
        // 元の色を分解
        uint8_t r = (originalColor >> 11) & 0x1F;
        uint8_t g = (originalColor >> 5) & 0x3F;
        uint8_t b = originalColor & 0x1F;
        
        // グラデーション背景色と自然にブレンド
        float bgY = (float)(y + row) / 240.0;
        uint8_t bgR = (uint8_t)(0 + (64 * bgY)) >> 3;
        uint8_t bgG = (uint8_t)(0 + (128 * bgY)) >> 2;
        uint8_t bgB = (uint8_t)(128 + (127 * bgY)) >> 3;
        
        uint8_t blendR = (uint8_t)(r * alpha + bgR * (1.0 - alpha));
        uint8_t blendG = (uint8_t)(g * alpha + bgG * (1.0 - alpha));
        uint8_t blendB = (uint8_t)(b * alpha + bgB * (1.0 - alpha));
        
        uint16_t fadeColor = (blendR << 11) | (blendG << 5) | blendB;
        tft.pushColor(fadeColor);
      } else {
        // 通常領域
        tft.pushColor(originalColor);
      }
    }
  }
  
  tft.endWrite();
}

// 時刻表示（グラデーション背景対応）
void drawTime(String currentTime) {
    // 時刻が変わった時のみ更新
    if (currentTime != lastDisplayTime) {
        // 背景のグラデーション色を再描画（時刻表示エリア）
        drawGradientArea(10, 220, 80, 20);
        
        // 新しい時刻を描画
        tft.setTextSize(2);
        tft.setTextColor(TFT_YELLOW);
        tft.drawString(currentTime, 10, 220);
        lastDisplayTime = currentTime;
    }
}

// 日付表示（グラデーション背景対応）
void drawDate(String currentDate) {
    // 日付が変わった時のみ更新  
    if (currentDate != lastDisplayDate) {
        // 背景のグラデーション色を再描画（日付表示エリア）
        drawGradientArea(100, 220, 120, 20);
        
        // 新しい日付を描画
        tft.setTextSize(2);
        tft.setTextColor(TFT_CYAN);
        tft.drawString(currentDate, 100, 220);
        lastDisplayDate = currentDate;
    }
}

// 温度表示（グラデーション背景対応）
void drawTemperature(float temp) {
  // 値が変わった時のみ更新
  if (abs(temp - lastTemp) > 0.1) {
    // 背景のグラデーション色を再描画（温度表示エリア）
    drawGradientArea(200, 35, 120, 25);
    
    // フォントサイズを明示的に設定
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE);
    tft.drawString(String(temp, 1) + " C", 200, 35);
    lastTemp = temp;
  }
}

// 速度表示（グラデーション背景対応）
void drawSpeed(float speed) {
  // 値が変わった時のみ更新
  if (abs(speed - lastSpeed) > 0.1) {
    // 背景のグラデーション色を再描画（速度表示エリア）
    drawGradientArea(200, 155, 120, 25);
    
    // フォントサイズを明示的に設定
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE);
    tft.drawString(String(abs(speed), 1), 200, 155);
    lastSpeed = speed;
  }
}