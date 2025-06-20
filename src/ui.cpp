#include <Arduino.h>
#include <TFT_eSPI.h>
#include "ui.hpp"
#include "characters/wink_close.h"
#include "characters/wink_hot.h"      // 追加：高温用キャラクター画像
#include "../include/temperature.hpp"
#include "../include/speed.hpp"
#include "../include/time.hpp"
#include "../include/ui/ui_display.hpp"
#include "../include/ui/ui_state.hpp"
#include "../include/ui/ui_temperature.hpp"

extern TFT_eSPI tft;

// 状態管理変数
bool uiInitialized = false;
float lastTemperature = -999.0;
float lastSpeed = -999.0;
String lastTime = "";
String lastDate = "";
bool characterDisplayed = false;
bool isHotCharacterMode = false;

// 温度連動背景色用の変数
float currentBackgroundTemp = 20.0;  // デフォルト温度
float lastBackgroundUpdateTemp = -999.0;


// CarBuddyタイトル表示
void drawCarBuddyTitle() {
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("CarBuddy", 25, 8);
}

void updateCarBuddyTitle() {
    drawTemperatureGradientArea(25, 8, 120, 16, currentBackgroundTemp);
    drawCarBuddyTitle();
}


void fadeInMainScreen() {
    for (int fade = 0; fade <= 7; fade++) {
        tft.fillScreen(TFT_BLACK);
        
        if (fade >= 2) {
            uint16_t fadeOverlay = tft.color565(fade * 8, fade * 8, fade * 8);
            for (int y = 0; y < 240; y += 4) {
                for (int x = 0; x < 320; x += 3) {
                    if ((x + y) % 6 == 0) {
                        tft.drawPixel(x, y, fadeOverlay);
                    }
                }
            }
        }
        
        uint16_t textColor = tft.color565(fade * 32, fade * 32, fade * 32);
        tft.setTextColor(textColor);
        
        if (fade >= 3) {
            tft.setTextSize(2);
            tft.drawString("CarBuddy", 25, 8);
        }
        
        tft.setTextSize(3);
        tft.drawString("Temp:", 200, 10);
        tft.drawString("Speed:", 200, 130);
        tft.drawString("km/h", 240, 180);
        
        delay(80);
    }
    
    drawTemperatureGradientBackground(currentBackgroundTemp);
    
    tft.setTextColor(TFT_WHITE);
    
    tft.setTextSize(2);
    tft.drawString("CarBuddy", 25, 8);
    
    tft.setTextSize(3);
    tft.drawString("Temp:", 200, 10);
    tft.drawString("Speed:", 200, 130);
    tft.drawString("km/h", 240, 180);
}

// === 温度連動キャラクター画像表示関数（新規追加） ===

// 温度に応じて適切なキャラクター画像配列を選択
const uint16_t* getCharacterImageArray(float temp) {
    if (temp >= 32.0) {
        return winkHotCharacterImage;    // 32℃以上は高温用画像
    } else {
        return winkCloseCharacterImage;  // 32℃未満は通常画像
    }
}

// キャラクター画像をフェードインで表示（温度連動版）
void drawCharacterImageWithFade(int x, int y) {
    const int originalSize = 160;
    const int newSize = 180;
    const float scale = (float)newSize / originalSize;
    
    // 現在の温度に応じた画像配列を取得
    const uint16_t* characterImage = getCharacterImageArray(currentBackgroundTemp);
    
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
        delay(60);
    }
}

// 通常のキャラクター画像表示（温度連動版）
void drawCharacterImage(int x, int y) {
    const int originalSize = 160;
    const int newSize = 180;
    const float scale = (float)newSize / originalSize;
    
    // 現在の温度に応じた画像配列を取得
    const uint16_t* characterImage = getCharacterImageArray(currentBackgroundTemp);
    
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

// キャラクター画像を縁ぼかし効果付きで表示（温度連動版）
void drawCharacterImageWithEdgeFade(int x, int y) {
    const int originalSize = 160;
    const int newSize = 180;
    const float scale = (float)newSize / originalSize;
    const int fadeWidth = 8;
    
    // 現在の温度に応じた画像配列を取得
    const uint16_t* characterImage = getCharacterImageArray(currentBackgroundTemp);
    
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
            
            // 縁からの距離を計算
            int distanceFromEdge = min(min(row, newSize - row - 1), min(col, newSize - col - 1));
            
            if (distanceFromEdge < fadeWidth) {
                // フェード処理
                float alpha = 0.3 + 0.7 * ((float)distanceFromEdge / fadeWidth);
                
                // 背景色を取得（温度連動グラデーション）
                float backgroundRatio = (float)(y + row) / 240.0;
                uint8_t topR, topG, topB, bottomR, bottomG, bottomB;
                getTemperatureColors(currentBackgroundTemp, &topR, &topG, &topB, &bottomR, &bottomG, &bottomB);
                
                uint8_t bgR = (uint8_t)(topR + ((bottomR - topR) * backgroundRatio));
                uint8_t bgG = (uint8_t)(topG + ((bottomG - topG) * backgroundRatio));
                uint8_t bgB = (uint8_t)(topB + ((bottomB - topB) * backgroundRatio));
                
                // 色の分解と合成
                uint8_t charR = (originalColor >> 11) & 0x1F;
                uint8_t charG = (originalColor >> 5) & 0x3F;
                uint8_t charB = originalColor & 0x1F;
                
                // アルファブレンド
                uint8_t finalR = (uint8_t)((charR << 3) * alpha + bgR * (1.0 - alpha)) >> 3;
                uint8_t finalG = (uint8_t)((charG << 2) * alpha + bgG * (1.0 - alpha)) >> 2;
                uint8_t finalB = (uint8_t)((charB << 3) * alpha + bgB * (1.0 - alpha)) >> 3;
                
                uint16_t finalColor = tft.color565(finalR << 3, finalG << 2, finalB << 3);
                tft.pushColor(finalColor);
            } else {
                // フェードなし
                tft.pushColor(originalColor);
            }
        }
    }
    
    tft.endWrite();
}

// ===== 差分描画対応の表示関数 =====

// 温度表示
void drawTemperature(float temp) {
    // 値が変わった時のみ更新
    if (abs(temp - lastTemperature) > 0.1) {
        // 背景の温度連動グラデーション色を再描画（温度表示エリア + タイトルエリア）
        drawTemperatureGradientArea(195, 30, 130, 35, currentBackgroundTemp);
        
        // CarBuddyタイトルも更新（背景色変化に対応）
        updateCarBuddyTitle();
        
        // 温度値による文字色の判定
        uint16_t tempTextColor = TFT_WHITE;
        if (temp >= 32.0) {
            tempTextColor = TFT_YELLOW;  // 高温時は警告として黄色
        }
        
        // フォントサイズを明示的に設定
        tft.setTextSize(3);
        tft.setTextColor(tempTextColor);
        tft.drawString(String(temp, 1) + " C", 200, 35);
        lastTemperature = temp;
    }
}

// 速度表示
void drawSpeed(float speed) {
    if (abs(speed - lastSpeed) > 0.1) {
        drawTemperatureGradientArea(195, 150, 130, 25, currentBackgroundTemp);
        
        tft.setTextSize(3);
        tft.setTextColor(TFT_WHITE);
        tft.drawString(String(speed, 1), 200, 155);
        lastSpeed = speed;
    }
}

// 時刻表示
void drawTime(String timeStr) {
    if (timeStr != lastTime) {
        drawTemperatureGradientArea(5, 215, 80, 25, currentBackgroundTemp);  // 幅を100→80に短縮
        
        tft.setTextSize(2);
        tft.setTextColor(TFT_YELLOW);
        tft.drawString(timeStr, 10, 220);
        lastTime = timeStr;
    }
}

// 日付表示
void drawDate(String dateStr) {
    if (dateStr != lastDate) {
        drawTemperatureGradientArea(90, 215, 130, 25, currentBackgroundTemp);
        
        tft.setTextSize(2);
        tft.setTextColor(TFT_CYAN);
        tft.drawString(dateStr, 95, 220);
        lastDate = dateStr;
    }
}

// キャラクター表示管理（温度連動版）
void drawCharacter() {
    // 温度によるキャラクターモード切り替えチェック
    bool shouldUseHotCharacter = (currentBackgroundTemp >= 32.0);
    
    // キャラクターが未表示、またはモードが変更された場合に再描画
    if (!characterDisplayed || (shouldUseHotCharacter != isHotCharacterMode)) {
        // 状態更新
        isHotCharacterMode = shouldUseHotCharacter;
        
        // キャラクター表示エリアの背景を先にクリア
        drawTemperatureGradientArea(10, 30, 180, 180, currentBackgroundTemp);
        
        // 新しいキャラクター画像を描画
        drawCharacterImageWithEdgeFade(10, 30);
        characterDisplayed = true;
        
        // デバッグ出力
        Serial.print("Character switched to: ");
        Serial.println(isHotCharacterMode ? "HOT mode (wink_hot)" : "NORMAL mode (wink_close)");
    }
}


// UI初期化
void drawUI() {
    Serial.println("Initializing UI...");
    
    updateBackgroundTemperature(20.0);
    
    fadeInMainScreen();
    
    drawCharacter();  // 温度連動キャラクター表示
    
    uiInitialized = true;
    Serial.println("UI initialization completed");
}