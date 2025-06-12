#include <Arduino.h>
#include <TFT_eSPI.h>
#include "ui.hpp"
#include "temperature.hpp"
#include "speed.hpp"
#include "time.hpp"
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
    Serial.println("=== Starting CarBuddy - Temperature Reactive Version ===");

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
    Serial.println("TFT initialized");

    // スプラッシュ画面表示
    showSplashScreen();

    // 温度センサー初期化（UI初期化前に温度を取得）
    Serial.println("Initializing temperature sensor...");
    initTemperatureSensor();
    
    // 初期温度取得して背景色を設定
    float initialTemp = getTemperature();
    updateBackgroundTemperature(initialTemp);
    
    // 初期背景を確実に描画
    drawTemperatureGradientBackground(initialTemp);
    
    Serial.print("Initial temperature: ");
    Serial.print(initialTemp);
    Serial.print("°C - Background color: ");
    if (initialTemp >= 32.0) {
        Serial.println("RED");
    } else if (initialTemp >= 30.0) {
        Serial.println("BLUE→RED (Transition)");
    } else {
        Serial.println("BLUE");
    }

    // UI初期描画（温度連動フェードイン）
    drawUI();
    drawCharacterImageWithEdgeFade(10, 30);
    Serial.println("UI initialized with temperature-reactive background");

    // 時刻システム初期化
    initTimeSystem();

    // 速度センサー初期化
    Serial.println("Initializing speed sensor...");
    bool speedSuccess = initSpeedSensor();
    Serial.println(speedSuccess ? "Speed sensor OK" : "Speed sensor FAILED");

    // 初期値表示
    float temp = getTemperature();
    drawTemperature(temp);  // この時点で背景色が温度に応じて設定される
    
    // 時刻表示
    drawTime(getCurrentTime());
    drawDate(getCurrentDate());
    
    Serial.println("=== Setup Complete - Temperature Reactive UI Active ===");
    Serial.println("Core 0: WiFi processing");
    Serial.println("Core 1: Display & sensors");
    Serial.println("Background color changes: <30°C=Blue, 30-32°C=Transition, ≥32°C=Red");
}

void loop() {
    // Core 1では表示・センサー処理のみ実行
    // WiFi処理はCore 0で並行実行中
    
    unsigned long currentTime = millis();
    
    // 速度更新（100ms間隔でスムーズに）
    if (currentTime - lastSpeedUpdate >= SPEED_UPDATE_INTERVAL) {
        float speed = getSpeed();
        drawSpeed(speed);
        lastSpeedUpdate = currentTime;
    }
    
    // 時刻更新（1秒間隔）
    if (currentTime - lastTimeUpdate >= TIME_UPDATE_INTERVAL) {
        drawTime(getCurrentTime());
        drawDate(getCurrentDate());
        lastTimeUpdate = currentTime;
    }
    
    // 温度更新（2秒間隔）
    if (currentTime - lastTempUpdate >= TEMP_UPDATE_INTERVAL) {
        float temp = getTemperature();
        drawTemperature(temp);  // この関数内で背景色が自動更新される
        lastTempUpdate = currentTime;
    }
    
    // 背景色の滑らかな更新チェック（500ms間隔）
    if (currentTime - lastBackgroundUpdate >= BACKGROUND_UPDATE_INTERVAL) {
        float currentTemp = getTemperature();
        
        // 温度変化時は必ず背景を更新（閾値を下げて確実に更新）
        if (abs(currentTemp - lastDisplayedBackgroundTemp) > 0.3) {  // 0.3℃変化で更新
            
            // 温度に応じた背景色変化をシリアルに出力（デバッグ用）
            String colorMode;
            if (currentTemp >= 32.0) {
                colorMode = "RED (HOT!)";
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
            
            // UI固定ラベル
            tft.setTextColor(TFT_WHITE);
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
            
            // 時刻表示
            tft.setTextSize(2);
            tft.setTextColor(TFT_YELLOW);
            tft.drawString(currentTimeStr, 10, 220);
            
            // 日付表示
            tft.setTextSize(2);
            tft.setTextColor(TFT_CYAN);
            tft.drawString(currentDateStr, 100, 220);
            
            // キャラクター描画
            drawCharacterImageWithEdgeFade(10, 30);
            
            // 4. 前回値を更新（次回の差分描画用）
            forceUpdateAllDisplayValues();  // 前回値をリセット
            setLastDisplayValues(currentTemp, currentSpeed, currentTimeStr, currentDateStr); // 新しい値を設定
            
            lastDisplayedBackgroundTemp = currentTemp;
        }
        
        lastBackgroundUpdate = currentTime;
    }
    
    // シリアル出力（1秒間隔）- デバッグ用
    if (currentTime - lastSerialUpdate >= SERIAL_UPDATE_INTERVAL) {
        float temp = getTemperature();
        float speed = getSpeed();
        String timeStr = getCurrentTime();
        
        Serial.print(timeStr); Serial.print(" | ");
        Serial.print("Temp: "); Serial.print(temp, 1); Serial.print("°C | ");
        Serial.print("Speed: "); Serial.print(speed, 1); Serial.print("km/h | ");
        Serial.print("Clients: "); Serial.print(getConnectedClientCount());
        
        // 温度警告表示
        if (temp >= 32.0) {
            Serial.print(" | ⚠️ HIGH TEMP!");
        } else if (temp >= 30.0) {
            Serial.print(" | 🌡️ Rising");
        }
        Serial.println();
        
        lastSerialUpdate = currentTime;
    }
    
    // 非常に短い待機（Core 1の負荷軽減）
    delay(1);
}