#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "webserver.hpp"
#include "time.hpp"
#include <time.h>

// 内部インスタンス
static WebServer server(80);
static bool serverRunning = false;

// HTTPハンドラー関数の前方宣言
void handleRoot();
void handleSetTime();
bool parseAndSetTime(String timeStr);

int getConnectedClientCount() {
    return WiFi.softAPgetStationNum();
}

bool isClientConnected() {
    return (getConnectedClientCount() > 0);
}

void initWebServer() {
    Serial.println("Initializing web server...");
    
    // WiFi完全初期化
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
    
    serverRunning = true;
    Serial.println("Web server started successfully");
}

void handleWebServerClient() {
    if (serverRunning) {
        server.handleClient();
    }
}

void stopWebServer() {
    if (serverRunning) {
        server.stop();
        WiFi.softAPdisconnect(true);
        serverRunning = false;
        Serial.println("Web server stopped");
    }
}

// HTTPハンドラー実装
void handleRoot() {
    String html = "<!DOCTYPE html>";
    html += "<html><head><title>CarBuddy Time Sync</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body { font-family: Arial; margin: 40px; text-align: center; background: #1a1a2e; color: white; }";
    html += "h1 { color: #16213e; text-shadow: 2px 2px 4px #000; }";
    html += "button { padding: 15px 30px; font-size: 18px; margin: 10px; background: #0f4c75; color: white; border: none; border-radius: 8px; cursor: pointer; }";
    html += "button:hover { background: #3282b8; }";
    html += ".time { font-size: 24px; margin: 20px; background: #16213e; padding: 20px; border-radius: 10px; }";
    html += ".status { margin: 20px; padding: 10px; border-radius: 5px; }";
    html += "</style></head><body>";
    html += "<h1>🚗 CarBuddy Time Sync</h1>";
    html += "<div class='time' id='currentTime'></div>";
    html += "<button onclick='syncTime()'>🕐 Sync Time</button>";
    html += "<div class='status' id='status'></div>";
    html += "<script>";
    html += "function updateTime() {";
    html += "const now = new Date();";
    html += "document.getElementById('currentTime').innerHTML = ";
    html += "'Current Time: ' + now.getFullYear() + '-' + ";
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
    html += "document.getElementById('status').innerHTML = '<p style=\"color: yellow;\">Syncing...</p>';";
    html += "fetch('/settime', {";
    html += "method: 'POST',";
    html += "headers: {'Content-Type': 'application/x-www-form-urlencoded'},";
    html += "body: 'time=' + timeStr";
    html += "})";
    html += ".then(response => response.text())";
    html += ".then(data => {";
    html += "document.getElementById('status').innerHTML = ";
    html += "'<p style=\"color: #4caf50;\">✅ ' + data + '</p>';";
    html += "setTimeout(() => document.getElementById('status').innerHTML = '', 3000);";
    html += "})";
    html += ".catch(error => {";
    html += "document.getElementById('status').innerHTML = ";
    html += "'<p style=\"color: #f44336;\">❌ Error: ' + error + '</p>';";
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
        Serial.print("Received time sync request: ");
        Serial.println(timeStr);
        
        if (parseAndSetTime(timeStr)) {
            // 時刻をEEPROMに保存
            saveCurrentTime();
            
            String response = "Time sync complete: " + timeStr;
            server.send(200, "text/plain", response);
            Serial.println("Time sync successful");
        } else {
            server.send(400, "text/plain", "Time format error");
            Serial.println("Time format error");
        }
    } else {
        server.send(400, "text/plain", "No time data received");
        Serial.println("Empty time sync request");
    }
}

bool parseAndSetTime(String timeStr) {
    struct tm timeinfo;
    int year, month, day, hour, minute, second;
    
    int parsed = sscanf(timeStr.c_str(), "%d-%d-%d %d:%d:%d", 
                       &year, &month, &day, &hour, &minute, &second);
    
    if (parsed != 6) {
        Serial.println("Failed to parse time string");
        return false;
    }
    
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;
    
    time_t timestamp = mktime(&timeinfo);
    struct timeval tv = { timestamp, 0 };
    settimeofday(&tv, NULL);
    
    Serial.print("Time set successfully to: ");
    Serial.println(timeStr);
    
    return true;
}