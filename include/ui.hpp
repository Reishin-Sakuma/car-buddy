#ifndef UI_HPP
#define UI_HPP

#include <TFT_eSPI.h>

void showSplashScreen();           // スプラッシュ画面表示
void drawUI();
void fadeInMainScreen();           // メイン画面フェードイン
void drawSpeed(float speed);
void drawCharacterImage(int x, int y);
void drawCharacterImageWithFade(int x, int y);  // キャラクターフェードイン
void drawTemperature(float temp);

#endif