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
#include "../include/ui/ui_data.hpp"

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


// UI初期化
void drawUI() {
    Serial.println("Initializing UI...");
    
    updateBackgroundTemperature(20.0);
    
    fadeInMainScreen();
    
    drawCharacter();  // 温度連動キャラクター表示
    
    uiInitialized = true;
    Serial.println("UI initialization completed");
}