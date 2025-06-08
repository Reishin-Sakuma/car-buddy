#ifndef UI_HPP
#define UI_HPP

#include <TFT_eSPI.h>

void showSplashScreen();                    // スプラッシュ画面表示
void drawUI();
void fadeInMainScreen();                    // メイン画面フェードイン
void drawSpeed(float speed);
void drawCharacterImage(int x, int y);
void drawCharacterImageWithFade(int x, int y);     // キャラクターフェードイン
void drawCharacterImageWithEdgeFade(int x, int y); // 縁フェード
void drawCharacterImageWithSoftEdge(int x, int y); // ソフト縁フェード
void drawTemperature(float temp);

#endif