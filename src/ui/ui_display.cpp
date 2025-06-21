#include <Arduino.h>
#include <TFT_eSPI.h>
#include "../../include/ui/ui_display.hpp"
#include "../../include/ui/ui_temperature.hpp"
#include "../../include/ui/ui_character.hpp"
#include "../../include/ui/ui_state.hpp"

extern TFT_eSPI tft;

// スプラッシュ画面表示
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

// メイン画面フェードイン
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
    
    // 温度連動背景を描画
    drawTemperatureGradientBackground(getCurrentBackgroundTemp());
    
    tft.setTextColor(TFT_WHITE);
    
    tft.setTextSize(2);
    tft.drawString("CarBuddy", 25, 8);
    
    tft.setTextSize(3);
    tft.drawString("Temp:", 200, 10);
    tft.drawString("Speed:", 200, 130);
    tft.drawString("km/h", 240, 180);
}

// CarBuddyタイトル描画
void drawCarBuddyTitle() {
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("CarBuddy", 25, 8);
}

// CarBuddyタイトル更新
void updateCarBuddyTitle() {
    drawTemperatureGradientArea(25, 8, 120, 16, getCurrentBackgroundTemp());
    drawCarBuddyTitle();
}

// UI全体初期化
void drawUI() {
    Serial.println("Initializing UI...");
    
    updateBackgroundTemperature(20.0);
    
    fadeInMainScreen();
    
    drawCharacter();  // 温度連動キャラクター表示
    
    setUIInitialized(true);
    Serial.println("UI initialization completed");
}