#include <Arduino.h>
#include <TFT_eSPI.h>
#include "ui.hpp"
#include "../include/temperature.hpp"
#include "../include/speed.hpp"
#include "../include/time.hpp"
#include "../include/ui/ui_display.hpp"
#include "../include/ui/ui_state.hpp"
#include "../include/ui/ui_temperature.hpp"
#include "../include/ui/ui_character.hpp"

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


// UI初期化
void drawUI() {
    Serial.println("Initializing UI...");
    
    updateBackgroundTemperature(20.0);
    
    fadeInMainScreen();
    
    drawCharacter();  // 温度連動キャラクター表示
    
    uiInitialized = true;
    Serial.println("UI initialization completed");
}