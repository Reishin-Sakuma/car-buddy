#include <Arduino.h>
#include <TFT_eSPI.h>
#include "../include/mode_manager.hpp"
#include "../include/clock.hpp"
#include "../include/ui.hpp"

extern TFT_eSPI tft;

// ===== 状態管理変数 =====
static DisplayMode currentMode = MODE_CHARACTER;  // 初期モードはキャラクター
static volatile long encoderPosition = 0;
static volatile int modeChangeCounter = 0;  // モード切り替え用カウンター

// エンコーダー状態
static volatile bool aState = false;
static volatile bool bState = false;
static volatile bool lastAState = false;

// デバウンス用
static unsigned long lastModeChangeTime = 0;
static unsigned long lastEncoderTime = 0;
static const unsigned long modeChangeDelay = 300;   // モード切り替えの最小間隔
static const unsigned long encoderDebounce = 50;    // エンコーダーデバウンス

// ===== 割り込みハンドラー =====
void IRAM_ATTR handleEncoderA() {
    unsigned long currentTime = millis();
    if (currentTime - lastEncoderTime < encoderDebounce) {
        return; // 短時間での変化は無視（チャタリング防止）
    }
    
    aState = digitalRead(ENCODER_A_PIN);
    bState = digitalRead(ENCODER_B_PIN);
    
    // 回転方向判定とモードカウンター更新
    if (aState != lastAState) {
        if (aState != bState) {
            encoderPosition++;
            modeChangeCounter++;    // 時計回り
        } else {
            encoderPosition--;
            modeChangeCounter--;    // 反時計回り
        }
        lastEncoderTime = currentTime;
    }
    lastAState = aState;
}

// ===== モードマネージャー初期化 =====
void initModeManager() {
    Serial.println("=== 3ピンロータリーエンコーダー モードマネージャー初期化開始 ===");
    
    // ピン初期化
    pinMode(ENCODER_A_PIN, INPUT_PULLUP);
    pinMode(ENCODER_B_PIN, INPUT_PULLUP);
    
    // 初期状態取得
    lastAState = digitalRead(ENCODER_A_PIN);
    
    // 割り込み設定（A相のみ）
    attachInterrupt(digitalPinToInterrupt(ENCODER_A_PIN), handleEncoderA, CHANGE);
    
    // アナログ時計初期化
    initAnalogClock();
    
    Serial.print("初期モード: ");
    Serial.println(getCurrentModeString());
    Serial.println("エンコーダーを回転させてモード切り替えをテストしてください");
    Serial.println("=== モードマネージャー初期化完了 ===");
}

// ===== モードマネージャー更新（メインループで呼び出し） =====
void updateModeManager() {
    static int lastModeChangeCounter = 0;
    unsigned long currentTime = millis();
    
    // モードカウンターの変化をチェック
    if (modeChangeCounter != lastModeChangeCounter && 
        currentTime - lastModeChangeTime > modeChangeDelay) {
        
        // 新しいモードを計算（カウンターの絶対値を使用）
        int newModeInt = abs(modeChangeCounter) % MODE_COUNT;
        DisplayMode newMode = (DisplayMode)newModeInt;
        
        // モードが実際に変わった場合のみ切り替え
        if (newMode != currentMode) {
            switchToMode(newMode);
            lastModeChangeTime = currentTime;
            
            Serial.print("エンコーダー回転検出: カウンター=");
            Serial.print(modeChangeCounter);
            Serial.print(" → モード=");
            Serial.println(getCurrentModeString());
        }
        
        lastModeChangeCounter = modeChangeCounter;
    }
}

// ===== モード取得 =====
DisplayMode getCurrentMode() {
    return currentMode;
}

String getCurrentModeString() {
    switch (currentMode) {
        case MODE_CHARACTER:
            return "キャラクター画像モード";
        case MODE_ANALOG_CLOCK:
            return "アナログ時計モード";
        default:
            return "不明なモード";
    }
}

// ===== モード切り替え =====
void switchToNextMode() {
    int nextModeInt = (int)currentMode + 1;
    if (nextModeInt >= MODE_COUNT) {
        nextModeInt = 0;  // 最初のモードに戻る
    }
    switchToMode((DisplayMode)nextModeInt);
}

void switchToPreviousMode() {
    int prevModeInt = (int)currentMode - 1;
    if (prevModeInt < 0) {
        prevModeInt = MODE_COUNT - 1;  // 最後のモードに移る
    }
    switchToMode((DisplayMode)prevModeInt);
}

void switchToMode(DisplayMode mode) {
    if (mode == currentMode) return;  // 同じモードなら何もしない
    
    DisplayMode oldMode = currentMode;
    currentMode = mode;
    
    Serial.print("🔄 モード切り替え: ");
    Serial.print(getCurrentModeString());
    Serial.print(" (");
    Serial.print((int)oldMode);
    Serial.print(" → ");
    Serial.print((int)currentMode);
    Serial.println(")");
    
    // 表示エリアをクリアして新しいモードを表示
    clearDisplayArea();
    updateDisplay();
}

// ===== 表示エリアクリア =====
void clearDisplayArea() {
    // キャラクター/時計表示エリアをクリア（10, 30から180x180の領域）
    tft.fillRect(10, 30, 180, 180, TFT_BLACK);
    Serial.println("📺 表示エリアをクリアしました");
}

// ===== 表示更新 =====
void updateDisplay() {
    switch (currentMode) {
        case MODE_CHARACTER:
            // 🔧 修正: 温度連動機能付きのキャラクター描画関数を呼び出し
            setClockVisible(false);  // アナログ時計を非表示
            drawCharacter();         // 温度連動キャラクター描画（位置は関数内で制御）
            Serial.println("👤 温度連動キャラクター画像を表示しました");
            break;
            
        case MODE_ANALOG_CLOCK:
            // アナログ時計を表示
            setClockVisible(true);   // アナログ時計を表示状態に
            setClockPosition(95, 120);  // 時計の中心位置設定（キャラエリアの中央）
            setClockSize(80);           // 時計のサイズ設定
            drawAnalogClock();          // アナログ時計描画
            Serial.println("🕐 アナログ時計を表示しました");
            break;
            
        default:
            Serial.println("❌ エラー: 不明な表示モードです");
            break;
    }
}

// ===== デバッグ情報表示 =====
void printEncoderDebugInfo() {
    Serial.print("エンコーダー位置: ");
    Serial.print(encoderPosition);
    Serial.print(", モードカウンター: ");
    Serial.print(modeChangeCounter);
    Serial.print(", 現在モード: ");
    Serial.println(getCurrentModeString());
}