#include <Arduino.h>
#include <TFT_eSPI.h>
#include "ui.hpp"
#include "characters/wink_close.h"
#include "characters/wink_hot.h"      // 追加：高温用キャラクター画像
#include "../include/temperature.hpp"
#include "../include/speed.hpp"
#include "../include/time.hpp"

extern TFT_eSPI tft;

// 状態管理変数
static bool uiInitialized = false;
static float lastTemperature = -999.0;
static float lastSpeed = -999.0;
static String lastTime = "";
static String lastDate = "";
static bool characterDisplayed = false;
static bool isHotCharacterMode = false;  // 追加：高温キャラクターモード状態

// 温度連動背景色用の変数
static float currentBackgroundTemp = 20.0;  // デフォルト温度
static float lastBackgroundUpdateTemp = -999.0;

// === 温度連動グラデーション色計算 ===

// 温度に応じた色を計算（高温時の緑色問題を完全修正）
void getTemperatureColors(float temp, uint8_t* topR, uint8_t* topG, uint8_t* topB, 
                         uint8_t* bottomR, uint8_t* bottomG, uint8_t* bottomB) {
    if (temp >= 32.0) {
        // 32℃以上：確実に赤色系グラデーション（濃い赤 → 明るい赤）
        *topR = 120;   *topG = 0;     *topB = 0;      // 上部：濃い赤
        *bottomR = 255; *bottomG = 60;  *bottomB = 60;   // 下部：明るい赤
    } else if (temp >= 30.0) {
        // 30-32℃：青から赤への確実な遷移（緑を完全に避ける）
        float ratio = (temp - 30.0) / 2.0;  // 0.0 → 1.0
        
        // 青色から赤色への直接補間（緑成分を最小限に）
        *topR = (uint8_t)(0 + (120 * ratio));      // 0 → 120
        *topG = (uint8_t)(0 + (0 * ratio));       // 0 → 0 (緑成分なし)
        *topB = (uint8_t)(128 * (1.0 - ratio));   // 128 → 0
        
        *bottomR = (uint8_t)(64 + (191 * ratio));  // 64 → 255
        *bottomG = (uint8_t)(128 * (1.0 - ratio) + (60 * ratio)); // 128 → 60 (緑成分を抑制)
        *bottomB = (uint8_t)(255 * (1.0 - ratio) + (60 * ratio)); // 255 → 60
    } else if (temp >= 25.0) {
        // 25-30℃：青系グラデーション
        *topR = 0; *topG = 20; *topB = 100;     
        *bottomR = 40; *bottomG = 80; *bottomB = 180;  
    } else {
        // 25℃未満：濃い青色グラデーション
        *topR = 0; *topG = 0; *topB = 128;    
        *bottomR = 64; *bottomG = 128; *bottomB = 255;
    }
}

// 温度連動グラデーション背景を描画
void drawTemperatureGradientBackground(float temp) {
    uint8_t topR, topG, topB, bottomR, bottomG, bottomB;
    getTemperatureColors(temp, &topR, &topG, &topB, &bottomR, &bottomG, &bottomB);
    
    for (int y = 0; y < 240; y++) {
        // グラデーション計算（0.0から1.0の範囲）
        float ratio = (float)y / 240.0;
        
        // 上部色から下部色への補間
        uint8_t r = (uint8_t)(topR + ((bottomR - topR) * ratio));
        uint8_t g = (uint8_t)(topG + ((bottomG - topG) * ratio));
        uint8_t b = (uint8_t)(topB + ((bottomB - topB) * ratio));
        
        // RGB565色に変換
        uint16_t color = tft.color565(r, g, b);
        
        // 1行分を描画
        tft.drawFastHLine(0, y, 320, color);
    }
}

// 指定領域のみ温度連動グラデーション背景を描画
void drawTemperatureGradientArea(int x, int y, int width, int height, float temp) {
    uint8_t topR, topG, topB, bottomR, bottomG, bottomB;
    getTemperatureColors(temp, &topR, &topG, &topB, &bottomR, &bottomG, &bottomB);
    
    for (int row = 0; row < height; row++) {
        float ratio = (float)(y + row) / 240.0;
        
        uint8_t r = (uint8_t)(topR + ((bottomR - topR) * ratio));
        uint8_t g = (uint8_t)(topG + ((bottomG - topG) * ratio));
        uint8_t b = (uint8_t)(topB + ((bottomB - topB) * ratio));
        
        uint16_t color = tft.color565(r, g, b);
        tft.drawFastHLine(x, y + row, width, color);
    }
}

// 温度連動背景色更新
void updateBackgroundTemperature(float temp) {
    currentBackgroundTemp = temp;
}

// 後方互換性のための関数
void drawGradientBackground() {
    drawTemperatureGradientBackground(currentBackgroundTemp);
}

void drawGradientArea(int x, int y, int width, int height) {
    drawTemperatureGradientArea(x, y, width, height, currentBackgroundTemp);
}

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

// === スプラッシュ画面とフェードイン ===

void showSplashScreen() {
    tft.fillScreen(TFT_BLACK);
    
    for (int fade = 0; fade <= 8; fade++) {
        tft.setTextColor(tft.color565(fade * 32, fade * 32, fade * 32));
        tft.setTextSize(4);
        tft.drawString("car-buddy", 50, 100);
        delay(300);
        
        if (fade < 8) {
            tft.fillRect(50, 100, 220, 32, TFT_BLACK);
        }
    }
    
    delay(1000);
    
    for (int fade = 8; fade >= 0; fade--) {
        tft.fillRect(50, 100, 220, 32, TFT_BLACK);
        tft.setTextColor(tft.color565(fade * 32, fade * 32, fade * 32));
        tft.setTextSize(4);
        tft.drawString("car-buddy", 50, 100);
        delay(200);
    }
    
    delay(500);
    tft.fillScreen(TFT_BLACK);
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

// ===== 状態管理関数 =====

// 前回値を強制更新（全体再描画後に使用）
void forceUpdateAllDisplayValues() {
    lastTemperature = -999.0;
    lastSpeed = -999.0;
    lastTime = "";
    lastDate = "";
    characterDisplayed = false;
}

// 前回値を設定（強制再描画後に使用）
void setLastDisplayValues(float temp, float speed, String timeStr, String dateStr) {
    lastTemperature = temp;
    lastSpeed = speed;
    lastTime = timeStr;
    lastDate = dateStr;
    characterDisplayed = true;
}

// 全体再描画
void forceFullRedraw(float temp) {
    if (abs(temp - lastBackgroundUpdateTemp) > 1.0) {
        Serial.println("Background temperature change detected - forcing full redraw");
        
        float currentSpeed = getSpeed();
        String currentTimeStr = getCurrentTime();
        String currentDateStr = getCurrentDate();
        
        drawTemperatureGradientBackground(temp);
        lastBackgroundUpdateTemp = temp;
        
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(2);
        tft.drawString("CarBuddy", 25, 8);
        
        tft.setTextSize(3);
        tft.drawString("Temp:", 200, 10);
        tft.drawString("Speed:", 200, 130);
        tft.drawString("km/h", 240, 180);
        
        forceUpdateAllDisplayValues();
        
        drawTemperature(temp);
        drawSpeed(currentSpeed);
        drawTime(currentTimeStr);
        drawDate(currentDateStr);
        drawCharacter();  // 温度連動キャラクター表示
        
        setLastDisplayValues(temp, currentSpeed, currentTimeStr, currentDateStr);
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