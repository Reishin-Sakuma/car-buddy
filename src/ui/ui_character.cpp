#include <Arduino.h>
#include <TFT_eSPI.h>
#include "../../include/ui/ui_character.hpp"
#include "../../include/ui/ui_temperature.hpp"
#include "../characters/wink_close.h"
#include "../characters/wink_hot.h"

extern TFT_eSPI tft;

// ui.cppの変数を参照
extern float currentBackgroundTemp;
extern bool isHotCharacterMode;

// 外部関数を参照（最新温度取得用）
extern float getTemperature();

// デバッグ用：現在の状態を表示する関数
void debugCharacterState() {
    float currentTemp = getTemperature();  // 最新温度を取得
    Serial.print("Debug - realTemp: ");
    Serial.print(currentTemp);
    Serial.print(", currentBackgroundTemp: ");
    Serial.print(currentBackgroundTemp);
    Serial.print(", isHotCharacterMode: ");
    Serial.print(isHotCharacterMode);
    Serial.print(", shouldUseHot: ");
    Serial.println(currentTemp >= 32.0);
}

// === 温度連動キャラクター画像表示関数 ===

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
    
    // 最新の温度に応じた画像配列を取得
    float currentTemp = getTemperature();
    const uint16_t* characterImage = getCharacterImageArray(currentTemp);
    
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
    
    // 最新の温度に応じた画像配列を取得
    float currentTemp = getTemperature();
    const uint16_t* characterImage = getCharacterImageArray(currentTemp);
    
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
    
    // 最新の温度に応じた画像配列を取得
    float currentTemp = getTemperature();
    const uint16_t* characterImage = getCharacterImageArray(currentTemp);
    
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

// キャラクター領域をクリア
void clearCharacterArea() {
    const int x = 10, y = 40, size = 180;  // y座標を50→40に修正
    drawTemperatureGradientArea(x, y, size, size, currentBackgroundTemp);
}

// キャラクター表示のメイン関数
void drawCharacter() {
    // 最新の温度を直接取得
    float currentTemp = getTemperature();
    
    // デバッグ出力
    debugCharacterState();
    
    // 温度変化に応じたキャラクター切り替えの確認（最新温度を使用）
    bool shouldUseHotCharacter = (currentTemp >= 32.0);
    
    // キャラクター切り替えが必要かチェック
    if (shouldUseHotCharacter != isHotCharacterMode) {
        Serial.println("Character mode change detected!");
        isHotCharacterMode = shouldUseHotCharacter;
        Serial.print("Character mode switched to: ");
        Serial.println(isHotCharacterMode ? "HOT mode (wink_hot)" : "NORMAL mode (wink_close)");
        
        // 必ず領域をクリアしてから描画
        clearCharacterArea();
        
        // キャラクター画像を縁ぼかし効果付きで表示
        drawCharacterImageWithEdgeFade(10, 40);
        
        Serial.println("Character redrawn due to temperature change");
    } else {
        // 初回描画または状態変化なしの場合
        if (!isHotCharacterMode && currentTemp < 32.0) {
            // 通常モードの確認表示
            Serial.println("Character drawn (normal mode)");
        } else if (isHotCharacterMode && currentTemp >= 32.0) {
            // 高温モードの確認表示
            Serial.println("Character drawn (hot mode)");
        }
        
        // 領域をクリアしてから描画（確実な表示のため）
        clearCharacterArea();
        drawCharacterImageWithEdgeFade(10, 40);
    }
}