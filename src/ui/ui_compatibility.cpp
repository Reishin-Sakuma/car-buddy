#include <Arduino.h>
#include "../../include/ui/ui_compatibility.hpp"
#include "../../include/ui/ui_temperature.hpp"

// 後方互換性のための関数（旧関数のラッパー）
void drawGradientBackground() {
    drawTemperatureGradientBackground(getCurrentBackgroundTemp());
}

void drawGradientArea(int x, int y, int width, int height) {
    drawTemperatureGradientArea(x, y, width, height, getCurrentBackgroundTemp());
}