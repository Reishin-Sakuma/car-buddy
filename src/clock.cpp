#include <Arduino.h>
#include <TFT_eSPI.h>
#include <math.h>
#include "../include/clock.hpp"
#include "../include/time.hpp"

extern TFT_eSPI tft;

// ===== アナログ時計の設定 =====
static int clockCenterX = 95;      // 時計の中心X座標 (キャラクター領域)
static int clockCenterY = 120;     // 時計の中心Y座標
static int clockRadius = 80;       // 時計の半径
static bool clockVisible = false;  // 時計の表示状態

// ===== アナログ時計の初期化 =====
void initAnalogClock() {
    Serial.println("アナログ時計機能を初期化しました");
    clockVisible = false;
}

// ===== 角度からX,Y座標を計算 =====
void polarToCartesian(int centerX, int centerY, int radius, float angleDegrees, int* x, int* y) {
    float angleRadians = (angleDegrees - 90) * PI / 180.0;  // 12時を0度とする
    *x = centerX + (int)(radius * cos(angleRadians));
    *y = centerY + (int)(radius * sin(angleRadians));
}

// ===== 時計の文字盤を描画 =====
void drawClockFace() {
    // 外枠の円
    tft.drawCircle(clockCenterX, clockCenterY, clockRadius, TFT_WHITE);
    tft.drawCircle(clockCenterX, clockCenterY, clockRadius - 1, TFT_WHITE);
    
    // 12時間の目盛りを描画
    for (int hour = 1; hour <= 12; hour++) {
        float angle = hour * 30.0;  // 30度 × 時間
        int outerX, outerY, innerX, innerY;
        
        // 外側の点
        polarToCartesian(clockCenterX, clockCenterY, clockRadius - 3, angle, &outerX, &outerY);
        // 内側の点
        polarToCartesian(clockCenterX, clockCenterY, clockRadius - 12, angle, &innerX, &innerY);
        
        // 時間の目盛り線（太め）
        tft.drawLine(outerX, outerY, innerX, innerY, TFT_WHITE);
        tft.drawLine(outerX + 1, outerY, innerX + 1, innerY, TFT_WHITE);
    }
    
    // 5分刻みの細かい目盛り
    for (int minute = 1; minute <= 60; minute++) {
        if (minute % 5 != 0) {  // 5分刻み以外
            float angle = minute * 6.0;  // 6度 × 分
            int outerX, outerY, innerX, innerY;
            
            polarToCartesian(clockCenterX, clockCenterY, clockRadius - 3, angle, &outerX, &outerY);
            polarToCartesian(clockCenterX, clockCenterY, clockRadius - 8, angle, &innerX, &innerY);
            
            tft.drawLine(outerX, outerY, innerX, innerY, TFT_LIGHTGREY);
        }
    }
    
    // 中央の点
    tft.fillCircle(clockCenterX, clockCenterY, 4, TFT_WHITE);
    tft.drawCircle(clockCenterX, clockCenterY, 4, TFT_BLACK);
}

// ===== 時針を描画 =====
void drawHourHand(int hour, int minute) {
    // 時針の角度計算 (12時間表示、分も考慮)
    float hourAngle = (hour % 12) * 30.0 + (minute * 0.5);
    int handX, handY;
    
    polarToCartesian(clockCenterX, clockCenterY, clockRadius - 25, hourAngle, &handX, &handY);
    
    // 時針を描画（太い線）
    tft.drawLine(clockCenterX, clockCenterY, handX, handY, TFT_WHITE);
    tft.drawLine(clockCenterX + 1, clockCenterY, handX + 1, handY, TFT_WHITE);
    tft.drawLine(clockCenterX, clockCenterY + 1, handX, handY + 1, TFT_WHITE);
    tft.drawLine(clockCenterX + 1, clockCenterY + 1, handX + 1, handY + 1, TFT_WHITE);
}

// ===== 分針を描画 =====
void drawMinuteHand(int minute) {
    float minuteAngle = minute * 6.0;  // 6度 × 分
    int handX, handY;
    
    polarToCartesian(clockCenterX, clockCenterY, clockRadius - 15, minuteAngle, &handX, &handY);
    
    // 分針を描画
    tft.drawLine(clockCenterX, clockCenterY, handX, handY, TFT_YELLOW);
    tft.drawLine(clockCenterX + 1, clockCenterY, handX + 1, handY, TFT_YELLOW);
}

// ===== 秒針を描画 =====
void drawSecondHand(int second) {
    float secondAngle = second * 6.0;  // 6度 × 秒
    int handX, handY;
    
    polarToCartesian(clockCenterX, clockCenterY, clockRadius - 10, secondAngle, &handX, &handY);
    
    // 秒針を描画（細い線、赤色）
    tft.drawLine(clockCenterX, clockCenterY, handX, handY, TFT_RED);
}

// ===== アナログ時計全体を描画 =====
void drawAnalogClock() {
    if (!clockVisible) return;
    
    // 現在時刻を取得
    String timeStr = getCurrentTimeString();
    
    // 時刻解析
    int hour = 0, minute = 0, second = 0;
    if (timeStr.length() >= 19) {  // "2024-06-29 14:30:25" 形式
        hour = timeStr.substring(11, 13).toInt();
        minute = timeStr.substring(14, 16).toInt();
        second = timeStr.substring(17, 19).toInt();
    } else {
        // タイムアウト等でデフォルト時刻の場合
        hour = 12;
        minute = 0;
        second = 0;
    }
    
    // 時計エリアをクリア（背景に合わせた色で）
    tft.fillCircle(clockCenterX, clockCenterY, clockRadius + 2, TFT_BLACK);
    
    // 文字盤を描画
    drawClockFace();
    
    // 針を描画（重なる順序に注意）
    drawHourHand(hour, minute);   // 時針（一番下）
    drawMinuteHand(minute);       // 分針（中間）
    drawSecondHand(second);       // 秒針（一番上）
    
    // 中央の点を再描画（針の上に）
    tft.fillCircle(clockCenterX, clockCenterY, 3, TFT_WHITE);
    
    // デバッグ情報
    Serial.print("アナログ時計更新: ");
    Serial.print(hour);
    Serial.print(":");
    Serial.print(minute);
    Serial.print(":");
    Serial.println(second);
}

// ===== 時計エリアをクリア =====
void clearClockArea() {
    // キャラクター描画エリアと同じ領域をクリア
    tft.fillRect(clockCenterX - clockRadius - 5, clockCenterY - clockRadius - 5, 
                 (clockRadius + 5) * 2, (clockRadius + 5) * 2, TFT_BLACK);
}

// ===== 時計の位置設定 =====
void setClockPosition(int centerX, int centerY) {
    clockCenterX = centerX;
    clockCenterY = centerY;
}

// ===== 時計のサイズ設定 =====
void setClockSize(int radius) {
    clockRadius = radius;
}

// ===== 時計の表示状態 =====
bool isClockVisible() {
    return clockVisible;
}

void setClockVisible(bool visible) {
    clockVisible = visible;
    Serial.print("アナログ時計表示状態: ");
    Serial.println(visible ? "ON" : "OFF");
}