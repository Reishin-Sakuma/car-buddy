#include <Arduino.h>
#include <TFT_eSPI.h>
#include "../../include/ui/ui_data.hpp"
#include "../../include/ui/ui_temperature.hpp"

extern TFT_eSPI tft;

// ui.cppの状態変数を参照
extern float lastTemperature;
extern float lastSpeed;
extern String lastTime;
extern String lastDate;
extern float currentBackgroundTemp;

// 外部関数の宣言（CarBuddyタイトル更新用）
extern void updateCarBuddyTitle();

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
        
        Serial.print("Temperature updated: ");
        Serial.println(temp);
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
        
        Serial.print("Speed updated: ");
        Serial.println(speed);
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
        
        Serial.print("Time updated: ");
        Serial.println(timeStr);
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
        
        Serial.print("Date updated: ");
        Serial.println(dateStr);
    }
}