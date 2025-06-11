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

unsigned long lastTempUpdate = 0;
unsigned long lastSpeedUpdate = 0;
unsigned long lastTimeUpdate = 0;
unsigned long lastSerialUpdate = 0;

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
    Serial.println("=== Starting CarBuddy - Modular Version ===");

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

    // UI初期描画（フェードイン）
    drawUI();
    drawCharacterImageWithEdgeFade(10, 30);
    Serial.println("UI initialized");

    // 時刻システム初期化
    initTimeSystem();

    // 温度センサー初期化
    Serial.println("Initializing temperature sensor...");
    initTemperatureSensor();

    // 速度センサー初期化
    Serial.println("Initializing speed sensor...");
    bool speedSuccess = initSpeedSensor();
    Serial.println(speedSuccess ? "Speed sensor OK" : "Speed sensor FAILED");

    // 初期値表示
    float temp = getTemperature();
    drawTemperature(temp);
    
    // 時刻表示
    drawTime(getCurrentTime());
    drawDate(getCurrentDate());
    
    Serial.println("=== Setup Complete - All modules loaded ===");
    Serial.println("Core 0: WiFi processing");
    Serial.println("Core 1: Display & sensors");
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
    
    // シリアル出力（1秒間隔）- デバッグ用
    if (currentTime - lastSerialUpdate >= SERIAL_UPDATE_INTERVAL) {
        float temp = getTemperature();
        float speed = getSpeed();
        String timeStr = getCurrentTime();
        
        Serial.print(timeStr); Serial.print(" | ");
        Serial.print("Temp: "); Serial.print(temp, 1); Serial.print("°C | ");
        Serial.print("Speed: "); Serial.print(speed, 1); Serial.print("km/h | ");
        Serial.print("Clients: "); Serial.println(getConnectedClientCount());
        
        lastSerialUpdate = currentTime;
    }
    
    // 温度更新（2秒間隔）
    if (currentTime - lastTempUpdate >= TEMP_UPDATE_INTERVAL) {
        float temp = getTemperature();
        drawTemperature(temp);
        lastTempUpdate = currentTime;
    }
    
    // 非常に短い待機（Core 1の負荷軽減）
    delay(1);
}