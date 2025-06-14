#include <Arduino.h>
#include <TFT_eSPI.h>
#include "ui.hpp"
#include "../include/temperature.hpp"
#include "../include/speed.hpp"
#include "../include/time.hpp"
#include "webserver.hpp"

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

    Serial.println("=== Sensors initialized ===");

    // スプラッシュ画面表示
    showSplashScreen();
    
    // メイン画面初期化
    drawUI();
    
    Serial.println("=== Setup completed - Starting main loop ===");
}

void loop() {
    unsigned long currentTime = millis();
    
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
            
            // === 背景とすべての要素を同期描画（ラグ解消） ===
            
            // 1. 現在値を事前に取得
            float currentSpeed = getSpeed();
            String currentTimeStr = getCurrentTime();
            String currentDateStr = getCurrentDate();
            
            // 2. 全画面背景を更新
            drawTemperatureGradientBackground(currentTemp);
            
            // 3. すぐに全ての要素を描画（ラグなし）
            
            // CarBuddyタイトル（キャラクター上部空白に）
            tft.setTextColor(TFT_WHITE);
            tft.setTextSize(2);
            tft.drawString("CarBuddy", 25, 8);
            
            // UI固定ラベル（元の位置に戻す）
            tft.setTextSize(3);
            tft.drawString("Temp:", 200, 10);     // 元の位置に戻す
            tft.drawString("Speed:", 200, 130);
            tft.drawString("km/h", 240, 180);     // 元の位置に戻す
            
            // 温度表示（文字色判定付き）
            uint16_t tempTextColor = TFT_WHITE;
            if (currentTemp >= 32.0) {
                tempTextColor = TFT_YELLOW;
            }
            tft.setTextSize(3);
            tft.setTextColor(tempTextColor);
            tft.drawString(String(currentTemp, 1) + " C", 200, 35);  // 元の位置に戻す
            
            // 速度表示
            tft.setTextSize(3);
            tft.setTextColor(TFT_WHITE);
            tft.drawString(String(abs(currentSpeed), 1), 200, 155);  // 元の位置に戻す
            
            // 時刻表示
            tft.setTextSize(2);
            tft.setTextColor(TFT_YELLOW);
            tft.drawString(currentTimeStr, 10, 220);
            
            // 日付表示
            tft.setTextSize(2);
            tft.setTextColor(TFT_CYAN);
            tft.drawString(currentDateStr, 100, 220);
            
            // キャラクター描画（元の位置に戻す）
            drawCharacterImageWithEdgeFade(10, 30);  // 元の位置に戻す
            
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

    // === 背景色更新チェック（温度以外での変化も検出） ===
    if (currentTime - lastBackgroundUpdate >= BACKGROUND_UPDATE_INTERVAL) {
        float currentTemp = getTemperature();
        
        // 小さな温度変化でも定期的に背景色更新をチェック
        if (abs(currentTemp - lastDisplayedBackgroundTemp) > 0.5) {
            updateBackgroundTemperature(currentTemp);
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
    }

    // === キャラクター表示 ===
    drawCharacter();

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
        Serial.println(getConnectedClientCount());
        
        lastSerialUpdate = currentTime;
    }

    // 少し待機してCPU負荷を軽減
    delay(10);
}