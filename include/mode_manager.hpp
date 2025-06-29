#ifndef MODE_MANAGER_HPP
#define MODE_MANAGER_HPP

#include <Arduino.h>

// ===== 表示モード定義 =====
enum DisplayMode {
    MODE_CHARACTER = 0,    // キャラクター画像モード
    MODE_ANALOG_CLOCK = 1, // アナログ時計モード
    MODE_COUNT = 2         // モード数（自動計算用）
};

// ===== 3ピンロータリーエンコーダー設定 =====
#define ENCODER_A_PIN  5    // A相 (Pin A)
#define ENCODER_B_PIN  12   // B相 (Pin B)
// コモン（Pin C）はGNDに接続

// ===== 関数宣言 =====
void initModeManager();
void updateModeManager();
DisplayMode getCurrentMode();
String getCurrentModeString();
void switchToNextMode();
void switchToPreviousMode();
void switchToMode(DisplayMode mode);

// ===== 表示切り替え関数 =====
void updateDisplay();
void clearDisplayArea();

#endif