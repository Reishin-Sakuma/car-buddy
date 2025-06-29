#include <Arduino.h>
#include <TFT_eSPI.h>
#include "ui.hpp"
#include "../include/temperature.hpp"
#include "../include/speed.hpp"
#include "../include/time.hpp"
#include "webserver.hpp"
#include "../include/mode_manager.hpp"
#include "../include/clock.hpp"

TFT_eSPI tft = TFT_eSPI();

// マルチコア用タスクハンドル
TaskHandle_t WiFiTask;

// 更新間隔の設定
const unsigned long TEMP_UPDATE_INTERVAL = 2000;   // 温度: 2秒
const unsigned long SPEED_UPDATE_INTERVAL = 100;   // 速度: 100ms
const unsigned long TIME_UPDATE_INTERVAL = 1000;   // 時刻: 1秒
const unsigned long SERIAL_UPDATE_INTERVAL = 1000; // シリアル出力: 1秒
const unsigned long BACKGROUND_UPDATE_INTERVAL = 500; // 背景色更新: 500ms（滑らかな変化）

unsigned long lastTempUpdate = 0;
unsigned long lastSpeedUpdate = 0;
unsigned long lastTimeUpdate = 0;
unsigned long lastSerialUpdate = 0;
unsigned long lastBackgroundUpdate = 0;

// 温度連動背景色用の変数
static float lastDisplayedBackgroundTemp = 20.0;

// WiFi専用タスク（Core 0で実行）
void WiFiTaskCode(void * pvParameters) {
    Serial.println("WiFi Task started on Core 0");
    
    for(;;) {
        // Webサーバー処理
        handleWebServerClient();
        
        // WiFi接続クライアント数を定期チェック
        static unsigned long lastClientCheck = 0;
        if (millis() - lastClientCheck > 10000) {  // 10秒ごと
            int clients = getConnectedClientCount();
            if (clients > 0) {
                Serial.print("WiFi clients connected: ");
                Serial.println(clients);
            }
            lastClientCheck = millis();
        }
        
        vTaskDelay(1);  // 1ms待機（他タスクにCPU譲る）
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== Starting CarBuddy - Temperature Reactive Version with Title ===");

    // Webサーバー初期化（WiFiスタック含む）
    initWebServer();
    
    // WiFi専用タスクをCore 0で起動
    xTaskCreatePinnedToCore(
        WiFiTaskCode,   // タスク関数
        "WiFiTask",     // タスク名
        10000,          // スタックサイズ
        NULL,           // パラメータ
        2,              // 優先度（高め）
        &WiFiTask,      // タスクハンドル
        0               // Core 0で実行
    );
    
    Serial.println("WiFi task created on Core 0");
    delay(500);  // WiFiタスク起動待機

    // TFT初期化
    tft.init();
    tft.setRotation(1);
    Serial.print("TFT size: ");
    Serial.print(tft.width());
    Serial.print(" x ");
    Serial.println(tft.height());

    // 各種センサー初期化
    initTemperatureSensor();
    initSpeedSensor();
    initTimeSystem();
    initModeManager();

    Serial.println("=== Sensors initialized ===");

    // スプラッシュ画面表示
    showSplashScreen();
    
    // メイン画面初期化
    drawUI();
    
    Serial.println("=== Setup completed - Starting main loop ===");
}


void loop() {
    unsigned long currentTime = millis();
    
    // === モード切り替え処理（最優先） ===
    updateModeManager();  // ロータリーエンコーダーの状態をチェック
    
    // === 温度更新 ===
    if (currentTime - lastTempUpdate >= TEMP_UPDATE_INTERVAL) {
        float currentTemp = getTemperature();
        updateBackgroundTemperature(currentTemp);
        
        // 背景色の大幅変化をチェック
        if (abs(currentTemp - lastDisplayedBackgroundTemp) > 1.0) {
            String colorMode;
            if (currentTemp >= 35.0) {
                colorMode = "RED (Hot)";
            } else if (currentTemp >= 30.0) {
                colorMode = "BLUE→RED (Transition)";
            } else {
                colorMode = "BLUE (Cool)";
            }
            
            Serial.print("Background color update: ");
            Serial.print(lastDisplayedBackgroundTemp, 1);
            Serial.print("°C → ");
            Serial.print(currentTemp, 1);
            Serial.print("°C → ");
            Serial.println(colorMode);
            
            // === 背景とすべての要素を同期描画（モード考慮版） ===
            
            // 1. 現在値を事前に取得（タイムアウト設定済みで高速）
            float currentSpeed = getSpeed();
            String currentTimeStr = getCurrentTime();  // 50msタイムアウトで高速取得
            String currentDateStr = getCurrentDate();  // 50msタイムアウトで高速取得
            
            // 2. 全画面背景を更新
            drawTemperatureGradientBackground(currentTemp);
            
            // 3. すぐに全ての要素を描画（ラグなし）
            
            // CarBuddyタイトル（キャラクター上部空白に）
            tft.setTextColor(TFT_WHITE);
            tft.setTextSize(2);
            tft.drawString("CarBuddy", 25, 8);
            
            // UI固定ラベル（元の位置に戻す）
            tft.setTextSize(3);
            tft.drawString("Temp:", 200, 10);
            tft.drawString("Speed:", 200, 130);
            tft.drawString("km/h", 240, 180);
            
            // 温度表示（文字色判定付き）
            uint16_t tempTextColor = TFT_WHITE;
            if (currentTemp >= 32.0) {
                tempTextColor = TFT_YELLOW;
            }
            tft.setTextSize(3);
            tft.setTextColor(tempTextColor);
            tft.drawString(String(currentTemp, 1) + " C", 200, 35);
            
            // 速度表示
            tft.setTextSize(3);
            tft.setTextColor(TFT_WHITE);
            tft.drawString(String(abs(currentSpeed), 1), 200, 155);
            
            // 時刻表示（通常色）
            tft.setTextSize(2);
            tft.setTextColor(TFT_YELLOW);
            tft.drawString(currentTimeStr, 10, 220);
            
            // 日付表示（通常色）
            tft.setTextSize(2);
            tft.setTextColor(TFT_CYAN);
            tft.drawString(currentDateStr, 95, 220);
            
            // 🔧 修正: モードに応じた表示切り替え（キャラクター直接描画を削除）
            updateDisplay();  // モードに応じてキャラクターまたは時計を描画
            
            // 4. 前回値を更新
            forceUpdateAllDisplayValues();
            setLastDisplayValues(currentTemp, currentSpeed, currentTimeStr, currentDateStr);
            
            lastDisplayedBackgroundTemp = currentTemp;
        } else {
            // 通常の差分描画
            drawTemperature(currentTemp);
        }
        
        lastTempUpdate = currentTime;
    }

    // === 背景色更新チェック（簡単版の修正） ===
    if (currentTime - lastBackgroundUpdate >= BACKGROUND_UPDATE_INTERVAL) {
        float currentTemp = getTemperature();
        
        // 小さな温度変化でも定期的に背景色更新をチェック
        if (abs(currentTemp - lastDisplayedBackgroundTemp) > 0.5) {
            updateBackgroundTemperature(currentTemp);
            
            // 🔧 修正: 既存のforceFullRedraw()を使用（ui_state.cppで既に修正済み）
            forceFullRedraw(currentTemp);
            
            lastDisplayedBackgroundTemp = currentTemp;
        }
        
        lastBackgroundUpdate = currentTime;
    }

    // === 速度更新 ===
    if (currentTime - lastSpeedUpdate >= SPEED_UPDATE_INTERVAL) {
        drawSpeed(getSpeed());
        lastSpeedUpdate = currentTime;
    }

    // === 時刻更新 ===
    if (currentTime - lastTimeUpdate >= TIME_UPDATE_INTERVAL) {
        drawTime(getCurrentTime());
        drawDate(getCurrentDate());
        lastTimeUpdate = currentTime;
        
        // アナログ時計モードの場合、時計も更新
        if (getCurrentMode() == MODE_ANALOG_CLOCK) {
            drawAnalogClock();  // 1秒ごとに時計を更新（秒針のため）
        }
    }

    // === シリアルデバッグ出力 ===
    if (currentTime - lastSerialUpdate >= SERIAL_UPDATE_INTERVAL) {
        float temp = getTemperature();
        float speed = getSpeed();
        String timeStr = getCurrentTime();
        String dateStr = getCurrentDate();
        
        Serial.print("Temp: ");
        Serial.print(temp, 1);
        Serial.print("°C, Speed: ");
        Serial.print(speed, 1);
        Serial.print(" km/h, Time: ");
        Serial.print(timeStr);
        Serial.print(", Date: ");
        Serial.print(dateStr);
        Serial.print(", WiFi clients: ");
        Serial.print(getConnectedClientCount());
        Serial.print(", 現在モード: ");
        Serial.println(getCurrentModeString());
        
        lastSerialUpdate = currentTime;
    }

    delay(10);
}