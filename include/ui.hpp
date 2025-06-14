#ifndef UI_HPP
#define UI_HPP

#include <Arduino.h>

// スプラッシュ画面とメイン画面の表示
void showSplashScreen();
void drawUI();
void fadeInMainScreen();

// 温度連動グラデーション背景
void drawTemperatureGradientBackground(float temp);
void drawTemperatureGradientArea(int x, int y, int width, int height, float temp);
void updateBackgroundTemperature(float temp);

// 後方互換性のための関数
void drawGradientBackground();
void drawGradientArea(int x, int y, int width, int height);

// CarBuddyタイトル表示関数（新規追加）
void drawCarBuddyTitle();
void updateCarBuddyTitle();

// キャラクター画像表示
void drawCharacterImageWithFade(int x, int y);
void drawCharacterImage(int x, int y);
void drawCharacterImageWithEdgeFade(int x, int y);

// 各種データ表示（差分描画対応）
void drawTemperature(float temp);
void drawSpeed(float speed);
void drawTime(String timeStr);
void drawDate(String dateStr);
void drawCharacter();

// 全体描画管理
void forceFullRedraw(float temp);
void forceUpdateAllDisplayValues();
void setLastDisplayValues(float temp, float speed, String timeStr, String dateStr);

#endif