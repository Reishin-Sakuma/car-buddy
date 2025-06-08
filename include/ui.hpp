#ifndef UI_HPP
#define UI_HPP

#include <TFT_eSPI.h>

void showSplashScreen();   // スプラッシュ画面表示
void drawUI();
void drawSpeed(float speed);
void drawCharacterImage(int x, int y);
void drawTemperature(float temp);

#endif