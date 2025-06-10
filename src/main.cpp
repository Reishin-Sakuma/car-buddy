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

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== Starting Talking Monitor with WiFi Time Sync ===");

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
    drawCharacterImageWithEdgeFade(10, 30);  // 弱めの四角い縁フェード
    Serial.println("UI initialized");

    // 時刻システム初期化
    initTimeSystem();

    // WiFiアクセスポイント設定
    Serial.println("Setting up WiFi Access Point...");
    WiFi.softAP("CarBuddy-WiFi", "carbuddy123");
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Webサーバー設定
    server.on("/", handleRoot);
    server.on("/settime", HTTP_POST, handleSetTime);
    server.enableCORS(true);
    server.begin();
    Serial.println("Web server started for time sync");

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
    
    Serial.println("=== Setup Complete ===");
}

void loop() {
    unsigned long currentTime = millis();
    
    // WiFiサーバー処理
    server.handleClient();
    
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
    
    // 時刻更新（1秒間隔）
    if (currentTime - lastTimeUpdate >= TIME_UPDATE_INTERVAL) {
        drawTime(getCurrentTime());
        drawDate(getCurrentDate());
        lastTimeUpdate = currentTime;
    }
    
    // シリアル出力（1秒間隔）
    if (currentTime - lastSerialUpdate >= SERIAL_UPDATE_INTERVAL) {
        float temp = getTemperature();
        float speed = getSpeed();
        String timeStr = getCurrentTime();
        
        Serial.print(timeStr); Serial.print(" | ");
        Serial.print("Temp: "); Serial.print(temp, 1); Serial.print("°C | ");
        Serial.print("Speed: "); Serial.print(speed, 1); Serial.println("km/h");
        
        lastSerialUpdate = currentTime;
    }
    
    // CPU負荷軽減のための短いディレイ
    delay(10);
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