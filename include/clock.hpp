#ifndef CLOCK_HPP
#define CLOCK_HPP

#include <Arduino.h>

// アナログ時計の初期化と描画
void initAnalogClock();
void drawAnalogClock();
void clearClockArea();

// 時計の設定
void setClockPosition(int centerX, int centerY);
void setClockSize(int radius);

// 時計の状態管理
bool isClockVisible();
void setClockVisible(bool visible);

#endif