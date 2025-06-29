#include <Arduino.h>
#include <TFT_eSPI.h>
#include "../../include/ui/ui_state.hpp"
#include "../../include/mode_manager.hpp"  // 🆕 追加: DisplayMode定義用

// 他のモジュールから参照する関数の宣言（暫定）
extern void drawTemperatureGradientBackground(float temp);
extern void drawTemperature(float temp);
extern void drawSpeed(float speed);
extern void drawTime(String timeStr);
extern void drawDate(String dateStr);
extern void drawCharacter();
extern String getCurrentTime();
extern String getCurrentDate();
extern float getSpeed();

// モード管理関数の宣言（mode_manager.hppから）
extern void updateDisplay();
extern DisplayMode getCurrentMode();
extern String getCurrentModeString();

// ui.cppの状態変数を参照（二重定義を避ける）
extern bool uiInitialized;
extern float lastTemperature;
extern float lastSpeed;
extern String lastTime;
extern String lastDate;
extern bool characterDisplayed;
extern bool isHotCharacterMode;
extern float lastBackgroundUpdateTemp;
extern TFT_eSPI tft;

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

// 既存のforceFullRedraw関数（モード考慮版に更新）
void forceFullRedraw(float temp) {
    extern float currentBackgroundTemp;
    
    if (abs(temp - lastBackgroundUpdateTemp) > 1.0) {
        Serial.println("Background temperature change detected - forcing full redraw");
        
        float currentSpeed = getSpeed();
        String currentTimeStr = getCurrentTime();
        String currentDateStr = getCurrentDate();
        
        drawTemperatureGradientBackground(temp);
        lastBackgroundUpdateTemp = temp;
        
        // 固定UI要素を再描画
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
        
        // 🔧 修正: モードを考慮したキャラクター/時計描画
        updateDisplay();  // キャラクターまたはアナログ時計をモードに応じて描画
        
        setLastDisplayValues(temp, currentSpeed, currentTimeStr, currentDateStr);
    }
}

// 🆕 新規追加: モード考慮版の全体再描画関数
void forceFullRedrawWithMode(float temp) {
    extern float currentBackgroundTemp;
    
    Serial.print("🔄 モード考慮版全体再描画開始 - 現在モード: ");
    Serial.println(getCurrentModeString());
    
    float currentSpeed = getSpeed();
    String currentTimeStr = getCurrentTime();
    String currentDateStr = getCurrentDate();
    
    drawTemperatureGradientBackground(temp);
    lastBackgroundUpdateTemp = temp;
    
    // 固定UI要素を再描画
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
    
    // モードに応じた表示切り替え
    updateDisplay();  // キャラクターまたはアナログ時計をモードに応じて描画
    
    setLastDisplayValues(temp, currentSpeed, currentTimeStr, currentDateStr);
    
    Serial.println("✅ モード考慮版全体再描画完了");
}

// ===== 状態取得機能 =====

bool isUIInitialized() {
    return uiInitialized;
}

void setUIInitialized(bool initialized) {
    uiInitialized = initialized;
}

// 前回値取得機能
float getLastTemperature() {
    return lastTemperature;
}

float getLastSpeed() {
    return lastSpeed;
}

String getLastTime() {
    return lastTime;
}

String getLastDate() {
    return lastDate;
}

bool isCharacterDisplayed() {
    return characterDisplayed;
}