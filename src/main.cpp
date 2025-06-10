#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include "ui.hpp"
#include "temperature.hpp"
#include "speed.hpp"
#include "time.hpp"

TFT_eSPI tft = TFT_eSPI();
WebServer server(80);

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

// WiFi時刻同期用の関数宣言
void handleSetTime();
void handleRoot();
bool parseAndSetTime(String timeStr);

// WiFi専用タスク（Core 0で実行）
void WiFiTaskCode(void * pvParameters) {
    Serial.println("WiFi Task started on Core 0");
    
    for(;;) {
        server.handleClient();
        
        // WiFi接続クライアント数を定期チェック
        static unsigned long lastClientCheck = 0;
        if (millis() - lastClientCheck > 10000) {  // 10秒ごと
            int clients = WiFi.softAPgetStationNum();
            Serial.print("WiFi clients: ");
            Serial.println(clients);
            lastClientCheck = millis();
        }
        
        vTaskDelay(1);  // 1ms待機（他タスクにCPU譲る）
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== Starting CarBuddy - MultiCore Version ===");

    // WiFi完全初期化（最優先）
    Serial.println("Initializing WiFi stack...");
    WiFi.mode(WIFI_OFF);
    delay(100);
    WiFi.mode(WIFI_AP_STA);
    delay(200);
    
    // TCP/IPスタック強制初期化
    WiFi.softAP("CarBuddy-WiFi", "carbuddy123");
    delay(500);  // AP安定化待機
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Webサーバー設定
    server.on("/", handleRoot);
    server.on("/settime", HTTP_POST, handleSetTime);
    server.enableCORS(true);
    server.begin();
    Serial.println("Web server started");
    
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
    
    Serial.println("=== Setup Complete - MultiCore Active ===");
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
    
    // シリアル出力（1秒間隔）- 正確なタイミング
    if (currentTime - lastSerialUpdate >= SERIAL_UPDATE_INTERVAL) {
        float temp = getTemperature();
        float speed = getSpeed();
        String timeStr = getCurrentTime();
        
        Serial.print(timeStr); Serial.print(" | ");
        Serial.print("Temp: "); Serial.print(temp, 1); Serial.print("°C | ");
        Serial.print("Speed: "); Serial.print(speed, 1); Serial.println("km/h");
        
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

// WiFi時刻同期用のハンドラー
void handleRoot() {
    String html = "<!DOCTYPE html>";
    html += "<html><head><title>CarBuddy Time Sync</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body { font-family: Arial; margin: 40px; text-align: center; }";
    html += "button { padding: 15px 30px; font-size: 18px; margin: 10px; }";
    html += ".time { font-size: 24px; margin: 20px; }";
    html += "</style></head><body>";
    html += "<h1>CarBuddy Time Sync</h1>";
    html += "<div class='time' id='currentTime'></div>";
    html += "<button onclick='syncTime()'>Sync Time</button>";
    html += "<div id='status'></div>";
    html += "<script>";
    html += "function updateTime() {";
    html += "const now = new Date();";
    html += "document.getElementById('currentTime').innerHTML = ";
    html += "now.getFullYear() + '-' + ";
    html += "String(now.getMonth() + 1).padStart(2, '0') + '-' + ";
    html += "String(now.getDate()).padStart(2, '0') + ' ' + ";
    html += "String(now.getHours()).padStart(2, '0') + ':' + ";
    html += "String(now.getMinutes()).padStart(2, '0') + ':' + ";
    html += "String(now.getSeconds()).padStart(2, '0');";
    html += "}";
    html += "function syncTime() {";
    html += "const now = new Date();";
    html += "const timeStr = now.getFullYear() + '-' + ";
    html += "String(now.getMonth() + 1).padStart(2, '0') + '-' + ";
    html += "String(now.getDate()).padStart(2, '0') + ' ' + ";
    html += "String(now.getHours()).padStart(2, '0') + ':' + ";
    html += "String(now.getMinutes()).padStart(2, '0') + ':' + ";
    html += "String(now.getSeconds()).padStart(2, '0');";
    html += "fetch('/settime', {";
    html += "method: 'POST',";
    html += "headers: {'Content-Type': 'application/x-www-form-urlencoded'},";
    html += "body: 'time=' + timeStr";
    html += "})";
    html += ".then(response => response.text())";
    html += ".then(data => {";
    html += "document.getElementById('status').innerHTML = ";
    html += "'<p style=\"color: green;\">OK: ' + data + '</p>';";
    html += "})";
    html += ".catch(error => {";
    html += "document.getElementById('status').innerHTML = ";
    html += "'<p style=\"color: red;\">Error: ' + error + '</p>';";
    html += "});";
    html += "}";
    html += "setInterval(updateTime, 1000);";
    html += "updateTime();";
    html += "</script></body></html>";
    
    server.send(200, "text/html", html);
}

void handleSetTime() {
    if (server.hasArg("time")) {
        String timeStr = server.arg("time");
        Serial.print("Received time: ");
        Serial.println(timeStr);
        
        if (parseAndSetTime(timeStr)) {
            // 時刻をEEPROMに保存
            saveCurrentTime();
            
            server.send(200, "text/plain", "Time sync complete: " + timeStr);
            Serial.println("Time sync successful");
        } else {
            server.send(400, "text/plain", "Time format error");
            Serial.println("Time format error");
        }
    } else {
        server.send(400, "text/plain", "No time data");
    }
}

bool parseAndSetTime(String timeStr) {
    struct tm timeinfo;
    int year, month, day, hour, minute, second;
    
    int parsed = sscanf(timeStr.c_str(), "%d-%d-%d %d:%d:%d", 
                       &year, &month, &day, &hour, &minute, &second);
    
    if (parsed != 6) return false;
    
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;
    
    time_t timestamp = mktime(&timeinfo);
    struct timeval tv = { timestamp, 0 };
    settimeofday(&tv, NULL);
    
    return true;
}