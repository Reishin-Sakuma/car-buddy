#ifndef UI_COMPATIBILITY_HPP
#define UI_COMPATIBILITY_HPP

#include <Arduino.h>

// 後方互換性のための関数（旧関数のラッパー）
void drawGradientBackground();
void drawGradientArea(int x, int y, int width, int height);

#endif