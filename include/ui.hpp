#ifndef UI_HPP
#define UI_HPP

#include <TFT_eSPI.h>

// === 既存の関数 ===
void showSplashScreen();                    // スプラッシュ画面表示
void drawUI();
void fadeInMainScreen();                    // メイン画面フェードイン
void drawSpeed(float speed);
void drawCharacterImage(int x, int y);
void drawCharacterImageWithFade(int x, int y);     // キャラクターフェードイン
void drawCharacterImageWithEdgeFade(int x, int y); // 縁フェード
void drawCharacterImageWithSoftEdge(int x, int y); // ソフト縁フェード
void drawTemperature(float temp);
void drawTime(String currentTime);         // 時刻表示
void drawDate(String currentDate);         // 日付表示

// === 温度連動背景色の新機能 ===
void drawTemperatureGradientBackground(float temp);   // 温度連動グラデーション背景
void drawTemperatureGradientArea(int x, int y, int width, int height, float temp); // エリア再描画
void updateBackgroundTemperature(float temp);         // 背景温度更新
void getTemperatureColors(float temp, uint8_t* topR, uint8_t* topG, uint8_t* topB, 
                         uint8_t* bottomR, uint8_t* bottomG, uint8_t* bottomB); // 温度色計算

// === 後方互換性のための既存関数（内部で温度連動版を呼び出し） ===
void drawGradientBackground();             // 基本グラデーション背景
void drawGradientArea(int x, int y, int width, int height); // エリア再描画

// === 強制更新機能（背景変更時の表示値復旧用） ===
void forceUpdateAllDisplayValues();        // 全表示値の強制更新
void forceBlueBackground();                // 強制青背景設定（テスト用）

#endif