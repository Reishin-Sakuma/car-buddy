#include <Arduino.h>
#include <TFT_eSPI.h>
#include "ui.hpp"
#include "temperature.hpp"
#include "speed.hpp"

TFT_eSPI tft = TFT_eSPI();

// 更新間隔の設定
const unsigned long TEMP_UPDATE_INTERVAL = 2000;  // 温度: 2秒
const unsigned long SPEED_UPDATE_INTERVAL = 100;   // 速度: 100ms
const unsigned long SERIAL_UPDATE_INTERVAL = 1000; // シリアル出力: 1秒

unsigned long lastTempUpdate = 0;
unsigned long lastSpeedUpdate = 0;
unsigned long lastSerialUpdate = 0;

void robustTFTInit() {
    Serial.println("Initializing TFT...");
    
    // 複数回初期化を試行
    for (int attempts = 0; attempts < 3; attempts++) {
        Serial.print("TFT init attempt: ");
        Serial.println(attempts + 1);
        
        tft.init();
        delay(100);
        
        tft.setRotation(1);
        delay(100);
        
        // テスト描画で動作確認
        tft.fillScreen(TFT_RED);
        delay(200);
        tft.fillScreen(TFT_GREEN);
        delay(200);
        tft.fillScreen(TFT_BLUE);
        delay(200);
        
        // 画面サイズ確認
        Serial.print("TFT size: ");
        Serial.print(tft.width());
        Serial.print(" x ");
        Serial.println(tft.height());
        
        if (tft.width() == 320 && tft.height() == 240) {
            Serial.println("TFT initialization successful!");
            return;
        }
        
        Serial.println("TFT initialization failed, retrying...");
        delay(500);
    }
    
    Serial.println("TFT initialization failed after 3 attempts!");
    // フォールバック：シリアルのみモード
    while(true) {
        Serial.println("TFT ERROR - Check connections");
        delay(5000);
    }
}

void setup() {
    delay(1000);  // 電源安定化待機
    Serial.begin(115200);
    Serial.println("=== Starting Talking Monitor ===");

    // TFT初期化
    tft.init();
    tft.setRotation(1);
    Serial.print("TFT size: ");
    Serial.print(tft.width());
    Serial.print(" x ");
    Serial.println(tft.height());
    Serial.println("TFT initialized");

    // 堅牢なTFT初期化
    robustTFTInit();

    // スプラッシュ画面表示
    showSplashScreen();

    // UI初期描画（フェードイン）
    drawUI();
    drawCharacterImageWithEdgeFade(10, 30);  // 弱めの四角い縁フェード
    Serial.println("UI initialized");

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
    
    Serial.println("=== Setup Complete ===");
}

void loop() {
    unsigned long currentTime = millis();
    
    // 温度更新（2秒間隔）
    if (currentTime - lastTempUpdate >= TEMP_UPDATE_INTERVAL) {
        float temp = getTemperature();
        drawTemperature(temp);
        lastTempUpdate = currentTime;
    }
    
    // 速度更新（100ms間隔でスムーズに）
    if (currentTime - lastSpeedUpdate >= SPEED_UPDATE_INTERVAL) {
        float speed = getSpeed();
        drawSpeed(speed);
        lastSpeedUpdate = currentTime;
    }
    
    // シリアル出力（1秒間隔）
    if (currentTime - lastSerialUpdate >= SERIAL_UPDATE_INTERVAL) {
        float temp = getTemperature();
        float speed = getSpeed();
        
        Serial.print("Temp: "); Serial.print(temp, 1); Serial.print("°C | ");
        Serial.print("Speed: "); Serial.print(speed, 1); Serial.println("km/h");
        
        lastSerialUpdate = currentTime;
    }
    
    // CPU負荷軽減のための短いディレイ
    delay(10);
}