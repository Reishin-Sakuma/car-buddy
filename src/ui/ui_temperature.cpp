#include <Arduino.h>
#include <TFT_eSPI.h>
#include "../../include/ui/ui_temperature.hpp"

extern TFT_eSPI tft;

// ui.cppの温度連動背景色変数を参照
extern float currentBackgroundTemp;

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

// 現在の背景温度取得
float getCurrentBackgroundTemp() {
    return currentBackgroundTemp;
}