#ifndef UI_TEMPERATURE_HPP
#define UI_TEMPERATURE_HPP

#include <Arduino.h>

// 温度連動機能
void getTemperatureColors(float temp, uint8_t* topR, uint8_t* topG, uint8_t* topB, 
                         uint8_t* bottomR, uint8_t* bottomG, uint8_t* bottomB);
void drawTemperatureGradientBackground(float temp);
void drawTemperatureGradientArea(int x, int y, int width, int height, float temp);
void updateBackgroundTemperature(float temp);

// 後方互換性のための関数
void drawGradientBackground();
void drawGradientArea(int x, int y, int width, int height);

// 現在の背景温度取得
float getCurrentBackgroundTemp();

#endif