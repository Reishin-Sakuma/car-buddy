#include <Arduino.h>
#include <TFT_eSPI.h>
#include "../../include/ui/ui_display.hpp"

extern TFT_eSPI tft;

// 元のui.cppにある関数・変数を参照（暫定）
extern void drawTemperatureGradientBackground(float temp);
extern void drawTemperatureGradientArea(int x, int y, int width, int height, float temp);
extern void updateBackgroundTemperature(float temp);
extern void drawCharacter();
extern float currentBackgroundTemp;
extern bool uiInitialized;

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
