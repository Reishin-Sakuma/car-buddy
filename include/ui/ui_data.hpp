#ifndef UI_DATA_HPP
#define UI_DATA_HPP

#include <Arduino.h>

// データ表示機能（差分描画対応）
void drawTemperature(float temp);
void drawSpeed(float speed);
void drawTime(String timeStr);
void drawDate(String dateStr);

#endif